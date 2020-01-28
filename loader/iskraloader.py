#!/usr/bin/python3

# STM32 ISKRAJS board firmware downloader
# On board must be USB MSD bootloader
# Use: python3 stmloader.py firmwarepath/firmwarename.bin
# License GPLv2, Copyright 2020 Yury Botov

import sys
import os
import subprocess
import string
import shutil
import time
import getpass
import platform

volume_name = 'ISKRAJS'  # magic volume name of USB MSD board

# test presence of firmware file

# TODO - made firmware signature check


def firmwareIsPresent(firmware):
    if os.path.exists(firmware):
        if os.path.isfile(firmware):
            print('Firmware ' + firmware + ' is present.')
            file_size = str(os.path.getsize(firmware))
            print('Firmware file size is ' + file_size + ' bytes.')
            return True
        else:
            print('Firmware ' + firmware + ' is not a file.')
            return False
    else:
        print('Firmware file ' + firmware + ' is not present.')
        return False

# search drive literal name by volume label


def findDriveByDriveLabel(driveLabel, drvArr):
    for dl in drvArr:
        try:
            if (os.path.isdir(dl) != 0):
                val = subprocess.check_output(['cmd', '/c vol ' + dl])
                if (driveLabel in str(val)):
                    return dl
        except Exception:
            print("Error: findDriveByDriveLabel(): exception")
            sys.exit(1)
    return None

# copy firmware to drive


def copyFirmware(src, dst):
    try:
        shutil.copy(src, dst)
    except shutil.SameFileError:
        print('Source and Destination is the same.')
        sys.exit(1)
    except IOError as txt:
        print('IOError' + str(txt))
        sys.exit(1)
    except Exception:
        print('Uploading error. Can`t copy firmware to board.')
        sys.exit(1)
    else:
        print('Upload done.')
        sys.exit(0)


def main():
    if len(sys.argv) != 2:
        if platform.system() == 'Windows':
            print('Usage details: stmloader.py x:\\filepath\\filename.bin')
        elif platform.system() == 'Linux':
            print('Usage details: stmloader.py /filepath/filename.bin')
        elif platform.system() == 'Darwin':
            print('Usage details: stmloader.py /filepath/filename.bin')
        else:
            print('Error: unknown OS platform')
        sys.exit(1)

    firmware = sys.argv[1]

    if not firmwareIsPresent(firmware):
        sys.exit(1)

    # wait when will be mounted drive named ISKRAJS (defined as volume_name),
    # copy firmware if ready, timeout if not present
    cnt = 0

    if platform.system() == 'Windows':
        while cnt < 10:
            # enumerate drives
            available_drives = [
                '%s:' % d for d in string.ascii_lowercase
                if os.path.exists('%s:' % d)
            ]
            # search drive with magic volumelabel
            volume = findDriveByDriveLabel(volume_name, available_drives)
            if volume is not None:
                print(
                    'Ok. USB MSC device is ready on drive ' + volume +
                    ' Try to upload firmware...'
                )
                copyFirmware(firmware, volume + '\\firmware.bin')
            time.sleep(1)
            cnt = cnt + 1

    elif platform.system() == 'Linux':
        user_name = getpass.getuser()
        search_path = '/media/' + user_name + '/' + volume_name
        while cnt < 10:
            if os.path.exists(search_path):
                if os.path.isdir(search_path):
                    print(
                        'Ok. USB MSC device is ready on ' +
                        search_path + '. Try to upload firmware...'
                    )
                    copyFirmware(firmware, search_path + '/firmware.bin')
            time.sleep(1)
            cnt = cnt + 1

    elif platform.system() == 'Darwin':
        search_path = '/Volumes/' + volume_name
        while cnt < 10:
            if os.path.exists(search_path):
                if os.path.isdir(search_path):
                    print(
                        'Ok. USB MSC device is ready on ' +
                        search_path + '. Try to upload firmware...'
                    )
                    copyFirmware(firmware, search_path + '/firmware.bin')
            time.sleep(1)
            cnt = cnt + 1
    else:
        print('Error: Unknown OS platform.')
        sys.exit(1)

    print('Can`t load firmware. USB MSC timeout.')
    sys.exit(1)


if __name__ == '__main__':
    main()
