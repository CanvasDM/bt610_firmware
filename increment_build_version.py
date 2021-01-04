import os
import logging
import log_wrapper
import sys

MAJOR_STR = "VERSION_MAJOR "
MINOR_STR = "VERSION_MINOR "
BUILD_STR = "VERSION_PATCH "

CONF_STR = "CONFIG_MCUBOOT_IMAGE_VERSION="

LOGS = False


def increment_build_version(argv) -> str:
    """
    Reads header file (argv[1]),
    increments build/patch number,
    and writes header file
    If arg[2] is present (prj.conf), then update version in project file.  This
    will cause a full rebuild.
    """
    logger = logging.getLogger('increment_build_version')
    logger.setLevel(logging.INFO)
    lst = []
    file_name = argv[1]
    major = "0"
    minor = "0"
    patch = "0"
    with open(file_name, 'r') as fin:
        logger.debug("Opened file " + file_name)
        for line in fin:
            default_append = True
            if MAJOR_STR in line:
                define, name, major = line.split()
            elif MINOR_STR in line:
                define, name, minor = line.split()
            if BUILD_STR in line:
                logger.debug("Found " + BUILD_STR)
                try:
                    define, name, patch = line.split()
                    if name == BUILD_STR.strip():
                        version = (int(patch) + 1) % 256
                        s = define + ' ' + BUILD_STR + str(version) + '\n'
                        lst.append(s)
                        logger.debug("New Version string " + s.strip('\n'))
                        default_append = False
                        patch = str(version)
                except:
                    logger.debug("Couldn't parse " +
                                 BUILD_STR.strip() + " line")

            if default_append:
                lst.append(line)

    if len(lst) > 0:
        with open(file_name, 'w') as fout:
            fout.writelines(lst)
            logger.debug("Wrote file " + file_name)

    vs = major + "." + minor + "." + patch
    logger.info(vs)

    if len(argv) > 2:
        update_zephyr_file(vs, argv[2])

    return vs


def update_zephyr_file(version_str: str, file_name: str):
    """
    Takes string output of increment_build_version and writes it to Zephyr
    configuration file so that mcuboot has the version information.
    """
    logger = logging.getLogger('increment_build_version')
    lst = []
    found = False
    config = CONF_STR + '"' + version_str + '"\n'
    with open(file_name, 'r') as fin:
        for line in fin:
            if CONF_STR in line:
                lst.append(config)
                found = True
            else:
                lst.append(line)

    if not found:
        lst.append('\n\n')
        lst.append(config)

    if len(lst) > 0:
        with open(file_name, 'w') as fout:
            fout.writelines(lst)
            logger.debug("Wrote file " + file_name)


if __name__ == "__main__":
    if LOGS:
        log_wrapper.setup(__file__, console_level=logging.DEBUG, file_mode='a')
    version = increment_build_version(sys.argv)
    if not LOGS:
        print(version)
