##
## Script to Build i.MX 8QXP MEK UEFI firmware
##
##
## Copyright (c) Microsoft Corporation. All rights reserved.
## Copyright 2019,2022 NXP
##
import os, sys
import stat
from optparse import OptionParser
import logging
import subprocess
import shutil
import struct
from datetime import datetime
from datetime import date
import time

from edk2toollib.utility_functions import GetHostInfo, RunCmd

from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toolext.environment import shell_environment
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.invocables.edk2_setup import SetupSettingsManager
from edk2toolext.invocables.edk2_setup import RequiredSubmodule
from edk2toolext.invocables.edk2_update import UpdateSettingsManager

#
# Supported Profiles and Settings based on the profile.  This allows easier grouping
# of settings.  Add var name as the key and value as value.
#
gProfile = {
    "SECURE": {"BLD_*_CONFIG_AUTH_VAR" : "TRUE", "BLD_*_CONFIG_SECURE_BOOT" : "TRUE",  "BLD_*_CONFIG_MEASURED_BOOT" : "TRUE"},
    "DEV": {"BLD_*_CONFIG_AUTH_VAR" : "FALSE", "BLD_*_CONFIG_SECURE_BOOT" : "FALSE",  "BLD_*_CONFIG_MEASURED_BOOT" : "FALSE", "BLD_*_CONFIG_OPTEE_PROFILE" : "FALSE"},
    "FRONTPAGE": {"BLD_*_CONFIG_AUTH_VAR" : "FALSE", "BLD_*_CONFIG_SECURE_BOOT" : "FALSE",  "BLD_*_CONFIG_OPTEE_PROFILE" : "FALSE",  "BLD_*_CONFIG_USB" :"TRUE","BLD_*_CONFIG_MEASURED_BOOT" : "FALSE", "BLD_*_CONFIG_FRONTPAGE" : "TRUE"},
    }

#--------------------------------------------------------------------------------------------------------
# Subclass the UEFI builder and add platform specific functionality.
#
class ImxPlatformBuilder(UpdateSettingsManager, SetupSettingsManager, BuildSettingsManager, UefiBuilder):

    def __init__(self):
        UefiBuilder.__init__(self)

        self.name = "IMX8QXP_MEK"

        self.package_path = os.path.dirname(os.path.abspath(__file__))
        self.workspace_path = os.path.dirname(os.path.dirname(self.package_path))
        self.require_repos = ('MU_BASECORE','Common/MU','Common/MU_TIANO', 'Common/MU_OEM_SAMPLE','Silicon/ARM/MU_TIANO')
        self.project_scope = ('imxfamily', 'imx8')

        self.module_packages = ('MU_BASECORE','Silicon/ARM/NXP', 'Common/MU','Common/MU_TIANO', 'Common/MU_OEM_SAMPLE','Silicon/ARM/MU_TIANO')

    def GetActiveScopes(self):
        ''' get scope '''
        scopes = self.project_scope

        if GetHostInfo().os.upper() == "LINUX":
            scopes += ("gcc_aarch64_linux",)
        return scopes

    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return self.workspace_path

    def GetPackagesPath(self):
        ws = self.GetWorkspaceRoot()
        pp = [os.path.join(ws, x) for x in self.module_packages]
        return pp

    def GetPackagesSupported(self):
        ''' return iterable of edk2 packages supported by this build.
        These should be edk2 workspace relative paths '''
        workspace_relative_path = os.path.relpath(self.package_path, self.workspace_path)
        return (workspace_relative_path,)

    def GetArchitecturesSupported(self):
        ''' return iterable of edk2 architectures supported by this build '''
        return ("ARM", "AARCH64")

    def GetTargetsSupported(self):
        ''' return iterable of edk2 target tags supported by this build '''
        return ("DEBUG", "RELEASE")

    def GetRequiredSubmodules(self):
        ''' get required repos '''
        submodules = []
        for submod in self.require_repos:
            submodules.append(RequiredSubmodule(submod))
        return submodules

    def GetName(self):
        return self.name

    def AddCommandLineOptions(self, parserObj):
        ''' Add command line options to the argparser '''
        pass

    def RetrieveCommandLineOptions(self, args):
        '''  Retrieve command line options from the argparser '''
        pass

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")

        # CHECK FOR PROFILE USAGE. SET TO DEV IF NONE IS SPECIFIED.
        profile = self.env.GetValue("PROFILE")
        if((profile == None) or (profile.upper() == "DEFAULT")):
            profile = "DEV"
        profile = profile.upper()
        logging.critical("BUILD PROFILE SET: %s" % profile)
        p = gProfile[profile]
        if(p != None):
            for k,v in p.items():
                logging.debug("Setting %s = %s by using profile %s" % (k, v, self.env.GetValue("PROFILE")))
                self.env.SetValue(k, v, "PROFILE VALUE")
        else:
            #catch case user requested a profile that doesn't exist
            logging.critical("Profile: %s doesn't exist." % profile)
            return -1
        #END OF PROFILE SUPPORT

        self.env.SetValue("BLD_*_CONFIG_HEADLESS", "FALSE", "Display surface will be drawn by default")

        # # Useful if working with Lauterbach JTAG. Will dump to console the complete
        # # command to copy and past into the Lauterbach command line to load symbols
        # # for each module.
        # # Use in conjunction with Informational debug messages:
        # # DEBUG_INFO      0x00000040
        self.env.SetValue("BLD_*_CONFIG_DUMP_SYMBOL_INFO", "TRUE", "On by default")
        # # Enable if PSCI is implemented
        self.env.SetValue("BLD_*_CONFIG_MPCORE", "TRUE", "On by default")
        # # Disable the USB stack by default. It currently has issues and need to be taken care of
        self.env.SetValue("BLD_*_CONFIG_USB", "FALSE", "currently has issues")
        # # Disable the PCIexpress stack by default.
        self.env.SetValue("BLD_*_CONFIG_PCIE", "TRUE", "On by default")
        # # States whether OPTEE boot flow is in effect or not. This has the following
        # # implications:
        # # - OPTEE must have been loaded by ATF.
        # # - Specific memory layout that is defined in the platform .dsc file.
        self.env.SetValue("BLD_*_CONFIG_OPTEE", "TRUE", "On by default")
        # # Allow collecting performance tracing from OPTEE hot code paths
        # # Performance summary results can be inspected with the shell app Dp.efi
        self.env.SetValue("BLD_*_CONFIG_OPTEE_PROFILE", "TRUE", "On by default")
        # # TPM stack for the fTPM
        self.env.SetValue("BLD_*_CONFIG_MEASURED_BOOT", "TRUE", "On by default")
        # # UEFI authenticated variable runtime services
        self.env.SetValue("BLD_*_CONFIG_AUTH_VAR", "TRUE", "On by default")
        # # If Platform Key (PK) is provisioned, then SecureBoot variables will get
        # # updated to reflect that SecureBoot is enabled. Otherwise, SecureBoot
        # # variables will indicate that the feature is disabled, and the OS will
        # # not boot apply SecureBoot policies
        self.env.SetValue("BLD_*_CONFIG_SECURE_BOOT", "TRUE", "On by default")
        # # Select to build project MU Frontpage with DFCI and the UEFI Shell
        self.env.SetValue("BLD_*_CONFIG_FRONTPAGE", "FALSE", "Off by default")
        self.env.SetValue("BUILDREPORTING", "TRUE", "Enabling build report")
        self.env.SetValue("BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Setting build report types")

        self.env.SetValue("ACTIVE_PLATFORM", "NXP/MX8QXP_MEK/MX8QXP_MEK.dsc", "Platform Hardcoded")
        self.env.SetValue("PRODUCT_NAME", "MX8QXP_MEK", "Platform Hardcoded")
        self.env.SetValue("TARGET_ARCH", "AARCH64", "Platform Hardcoded")
        self.env.SetValue("TOOL_CHAIN_TAG", "GCC5", "Platform Hardcoded")
        self.env.SetValue("BLD_*_BUILDID_STRING", "1.1.1", "Computed during Build Process")

        # Without this, GCC with use non-ascii quotes
        os.environ["LC_CTYPE"] = "C"

        # check to see if full path already configured
        if shell_environment.GetEnvironment().get_shell_var("GCC5_AARCH64_PREFIX") is not None:
            logging.warn("GCC5_AARCH64_PREFIX is already set.")

        else:
            # now check for install dir.  If set then set the Prefix
            install_path = shell_environment.GetEnvironment(
            ).get_shell_var("GCC5_AARCH64_INSTALL")
            if install_path is None:
                return 0

            # make GCC5_AARCH64_PREFIX to align with tools_def.txt
            prefix = os.path.join(install_path, "bin", "aarch64-linux-gnu-")
            shell_environment.GetEnvironment().set_shell_var("GCC5_AARCH64_PREFIX", prefix)

        return 0

    def SetPlatformEnvAfterTarget(self):
        logging.debug("PlatformBuilder SetPlatformEnvAfterTarget")

        value = os.path.join(self.env.GetValue("BUILD_OUT_TEMP"), self.env.GetValue("TOOL_CHAIN_TAG"))
        self.env.SetValue("BUILD_OUTPUT_BASE", value, "Computed in MX8QXP_MEK Build")

        # Output FD file
        key = "OUTPUT_FD"
        value = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "FV", "ClientBios.fd")
        self.env.SetValue(key, value, "Computed in MX8QXP_MEK Build")

        # Output ROM directory
        key = "OUTPUT_ROM_DIR"
        value = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "ROM")
        self.env.SetValue(key, value, "Computed in MX8QXP_MEK Build")

        # Output FV file
        key = "OUTPUT_FV"
        value = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "FV", "FV_UNCOMPRESSED.Fv")
        self.env.SetValue(key, value, "Computed in MX8QXP_MEK Build")


        # Capsule
        self.env.SetValue("BUILDCAPSULE", "FALSE", "default to not build capsule")
        self.env.SetValue("TRANSBUILDCAPSULE", "FALSE", "default to not build transition capsule")
        return 0



    def PlatformPostBuild(self):
        return 0


    #------------------------------------------------------------------
    #
    # Method to do stuff pre build.
    # This is part of the build flow.
    # Currently do nothing.
    #
    #------------------------------------------------------------------
    def PlatformPreBuild(self):
        return 0

    #------------------------------------------------------------------
    #
    # Method for the platform to check if a gated build is needed
    # This is part of the build flow.
    # return:
    #  True -  Gated build is needed (default)
    #  False - Gated build is not needed for this platform
    #------------------------------------------------------------------
    def PlatformGatedBuildShouldHappen(self):
        return False

    def ComputeVersionValue(self):
        BuildType = self.env.GetValue("TARGET")
        BuildId = self.env.GetBuildValue("BUILDID")
        if(BuildId == None):
            # Get milestone
            lMilestone = self.env.GetValue("BUILD_MILESTONE")
            if(lMilestone == None):
                lMilestone = gMilestone

            # Get build type
            if(BuildType.upper().strip() == "DEBUG"):
                bt = BuildTypeIdBase + BuildTypeIdDebug
            elif(BuildType.upper().strip() == "RELEASE"):
                bt = BuildTypeIdBase + BuildTypeIdRelease
            else:
                bt = BuildTypeIdBase + BuildTypeIdOther

            # Call library to compute the version
            (ec, buildint) = self.CommonBuild.ComputeVersionInt(gProductId, lMilestone, bt)
            if(ec != 0):
                logging.debug("Failed BuildId computation")
                return ec

            #set build id value
            self.env.SetValue("BLD_*_BUILDID", str(buildint), "BuildId Computed")
        return 0
        # PALINDROME CHANGE


    def ValidateVersionValue(self):
        #
        # Validate the version value (32 bit encoded value)
        # Convert to string following the rules of the platform version string format (platform specific)
        # Print out the info.
        #

        #
        # This functions logic follows Eric Lee's defined Build ID definition
        # used on Project H, C, P.  To change product/milestone NO logic changes are needed as
        # they are set in global vars at the top of this file and  date is computed on the fly.
        #
        BuildId = self.env.GetBuildValue("BUILDID")
        if(BuildId == None):
            logging.debug("Failed to find a BuildId")
            return -1
        #Use KBL common build code to do version validation.
        (ec, ver, mil) = self.CommonBuild.ValidateVersionValue(BuildId, gProductId)
        if(ec != 0):
            logging.debug("Failed BuildId validation")
            return ec

        self.env.SetValue("BLD_*_BUILDID_STRING", ver, "Computed during Build Process")
        self.env.SetValue("BUILD_MILESTONE", str(mil), "Setting milestone based on string decode.  Might get rejected if already set which is fine.")

        if str(mil) != self.env.GetValue("BUILD_MILESTONE"):
            logging.critical("Milestone requested by build doesn't match milestone encoded in build id")
            logging.debug("Milestone from build id: %s" % str(mil))
            logging.debug("Milestone requested: %s" % self.env.GetValue("BUILD_MILESTONE"))
            return -1

        return 0

    #
    # Platform defined flash method
    #
    def PlatformFlashImage(self):
        raise Exception("Flashing not supported")


if __name__ == "__main__":
    import argparse
    import sys
    from edk2toolext.invocables.edk2_update import Edk2Update
    from edk2toolext.invocables.edk2_setup import Edk2PlatformSetup
    from edk2toolext.invocables.edk2_platform_build import Edk2PlatformBuild
    print("Invoking Stuart")
    print("     ) _     _")
    print("    ( (^)-~-(^)")
    print("__,-.\_( 0 0 )__,-.___")
    print("  'W'   \   /   'W'")
    print("         >o<")
    SCRIPT_PATH = os.path.relpath(__file__)
    parser = argparse.ArgumentParser(add_help=False)
    parse_group = parser.add_mutually_exclusive_group()
    parse_group.add_argument("--update", "--UPDATE",
                             action='store_true', help="Invokes stuart_update")
    parse_group.add_argument("--setup", "--SETUP",
                             action='store_true', help="Invokes stuart_setup")
    args, remaining = parser.parse_known_args()
    new_args = ["stuart", "-c", SCRIPT_PATH]
    new_args = new_args + remaining
    sys.argv = new_args
    if args.setup:
        print("Running stuart_setup -c " + SCRIPT_PATH)
        Edk2PlatformSetup().Invoke()
    elif args.update:
        print("Running stuart_update -c " + SCRIPT_PATH)
        Edk2Update().Invoke()
    else:
        print("Running stuart_build -c " + SCRIPT_PATH)
        Edk2PlatformBuild().Invoke()
