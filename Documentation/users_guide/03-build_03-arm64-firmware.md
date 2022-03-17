Building ARM64 Firmware {#build-arm64-firmware}
----

This chapter describes the process of setting up a build-environment to build the latest firmware and update the firmware on development board.

### Setting up your build environment

 1. Start Linux environment such as:
      * Dedicated Linux system
      * Linux Virtual Machine
      * Windows Subsystem for Linux ([WSL setup instructions](https://docs.microsoft.com/en-us/windows/wsl/install-win10))
        
        > Note: W<os_version>-imx-windows-bsp-<build_date>.zip was validated with Ubuntu 20.04 in WSL and also standalone Ubuntu.

 2. Install or update build tools. On Ubuntu run the following commands from shell.
    
    ```bash
    sudo apt-get update
    sudo apt-get upgrade
    
    # Ubuntu 18.04 and possibly other distributions might require newer version of mono than present in official repository. This could be solved by adding mono repository to the system.
    # Process for Ubuntu 18.04 taken from https://www.mono-project.com/download/stable/#download-lin:
    # sudo apt install gnupg ca-certificates
    # sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
    # # Optionally key could be downloaded to a file and added manually by 'apt-key add <keyfile>'.
    # # Now that certificate is installed we can add official mono repository to repository list.
    # echo "deb https://download.mono-project.com/repo/ubuntu stable-bionic main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
    # sudo apt update
        
    sudo apt-get install attr build-essential python3.8 python3.8-dev python3.8-venv device-tree-compiler bison flex swig iasl uuid-dev wget git bc libssl-dev zlib1g-dev python3-pip mono-devel gawk
    # The mu_project currently requires python 3.8 and higher. Create a new python environment for the build and activate it.
    python3.8 -m venv ~/venv/win_fw_build
    source ~/venv/win_fw_build/bin/activate
    
    # Get cross compiler
    pushd ~
    wget https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/aarch64-linux-gnu/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu.tar.xz
    tar xf gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu.tar.xz
    rm gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu.tar.xz
    # The cross compiler prefix is required to be exported later into AARCH64_TOOLCHAIN_PATH variable.
    # export AARCH64_TOOLCHAIN_PATH=~/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
    popd
    
    # Following steps require to be done inside BSP_DIR extracted from W<os_version>-imx-windows-bsp-<build_date>.zip.
    cd <BSP_DIR>
    pushd mu_platform_nxp
    pip3 install -r requirements.txt --upgrade

    # The pycrypto package is obsolete. Remove it if installed
    sudo apt remove python3-crypto # installed using apt
    pip3 uninstall pycrypto # installed using pip3
    # And install pycryptodome
    pip3 install pycryptodome
    python3 NXP/MCIMX8M_EVK_4GB/PlatformBuild.py --setup # If facing problems during UEFI build use --force to clean the environment. Commit or stash all your changes first, command performs git reset --hard.
    python3 NXP/MCIMX8M_EVK_4GB/PlatformBuild.py --update # If command fails try reinstalling mono from its official repository as noted above.
    popd
    ```

### Building the firmware

This chapter assumes the BSP sources have been prepared as described in [Obtaining BSP sources][] chapter.

To build the boot firmware export AARCH64_TOOLCHAIN_PATH and execute the `buildme64.sh -b <BOARD_NAME> -t all [-clean]` script provided in the root of BSP sources. The `buildme64.sh` script bundled in BSP also copies `flash.bin` and `uefi.fit` into `<BSP_DIR>/imx-windows-iot/component/firmware/<board_name>` to allow for partial (only UEFI or U-boot) rebuild.

In case of BSP update, you may need to run `python3 NXP/MCIMX8M_EVK_4GB/PlatformBuild.py --setup --force` inside mu_platform_nxp to get clean and up-to-date MU build environment. Make sure to stash or commit your changes. Command performs `git reset --hard`. 

```bash
# export AARCH64_TOOLCHAIN_PATH pointing at the cross compiler
#   eg. export AARCH64_TOOLCHAIN_PATH=~/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# Get into virtual environment
 source ~/venv/win_fw_build/bin/activate
./buildme64.sh -b 8M -t all -c -fw
```
 
To deploy `firmware.bin` to the i.MX development board follow the process described in [Deploy boot firmware][].

### Common causes of build errors:
 1. ImportError: No module named Crypto.PublicKey.
 	  - This error is encountered when pycryptodome module is missing. Optionally obsolete pycrypto needs to be removed.
 2. Unable to enter directory. Directory doesn't exist.
 	  - We have run into this problem in case gitmodules were not downloaded completely (eg. MSRSec is empty) or `cst` or `firmware` directories are missing. Try repeating the [Obtaining BSP sources][] step by step.
 3. The build fails in WSL while the BSP is located somewhere in `/mnt/c` of the WSL.
 	  - Try `setfattr -n system.wsl_case_sensitive -v 1 <BSP_DIR>`. OP-Tee also requires symbolic links. We have been able to build boot firmware in `/mnt/c/` on Windows os version 1909. Workaround is to copy the BSP to WSL filesystem eg. to HOME.
 4. RuntimeError: SDE is not current. Please update your env before running this tool.
      - Try to update mono_devel package and repeat setup and update steps for PlatformBuild.py in [Setting up your build environment][].
 5. cp: cannot stat '../../firmware-imx/firmware/ddr/synopsys/lpddr4_pmu_train_*.bin'
      - Please make sure the firmware-imx obtained in [Obtaining BSP sources][] chapter is placed inside the BSP in way the following directories exist `<BSP_DIR>/firmware-imx/firmware/ddr/synopsys`.
