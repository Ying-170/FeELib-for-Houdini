
import os


def do_cmake():
    # cmake -G "Visual Studio 16 2019" .. -DCMAKE_PREFIX_PATH="C:\Program Files\Side Effects Software\Houdini 19.0.498\toolkit\cmake"
    # cmake -G "Visual Studio 17 2022" .. -DCMAKE_PREFIX_PATH="C:\Program Files\Side Effects Software\Houdini 19.0.498\toolkit\cmake"

    inputVSVersion = input('plz input your Visual Studio Version like 19 or 22 (default is 19)')
    inputVSVersion = inputVSVersion.strip()

    try:
        inputVSVersion = int(inputVSVersion)
        if inputVSVersion == 15:
            parm_VSVersion = "Visual Studio 14 2015"
        elif inputVSVersion == 17:
            parm_VSVersion = "Visual Studio 15 2017"
        elif inputVSVersion == 19:
            parm_VSVersion = "Visual Studio 16 2019"
        elif inputVSVersion == 22:
            parm_VSVersion = "Visual Studio 17 2022"
        else:
            parm_VSVersion = "Visual Studio 16 2019"
    except:
        parm_VSVersion = "Visual Studio 16 2019"

    for houdini_version in ("18.5", "19.0", "19.5"):
        HFS_envKey = "HFS"
        parm_DCMAKE_PREFIX_PATH = os.environ[HFS_envKey]

        if not os.path.exists(parm_DCMAKE_PREFIX_PATH):
            continue

        fileRootPath = "./../SOP/"
        fileRootPath = os.path.realpath(fileRootPath) + '/'

        for folderName in os.listdir(fileRootPath):
            relFilePath = fileRootPath + folderName
            absFilePath = os.path.realpath(relFilePath)
            if os.path.isfile(absFilePath):
                continue

            absBuildPath = "{0}/build".format(absFilePath).replace('\\', '/')

            if os.path.exists(absBuildPath):
                # print(os.listdir(absBuildPath))
                if os.listdir(absBuildPath):
                    continue
            else:
                os.mkdir(absBuildPath)

            # print(absBuildPath)
            os.chdir(absBuildPath)
            os.system("cmake -G \"{0}\" ..\"{1}\"".format(parm_VSVersion, parm_DCMAKE_PREFIX_PATH))
            # os.system("pause")

        # command = ''
        # for folderName in os.listdir(fileRootPath):
        #     absFilePath = fileRootPath + folderName
        #     if os.path.isfile(absFilePath):
        #         continue

        #     # print(absFilePath)
        #     command += "\n"
        #     command += "cd \".\\SOP\\{0}\\build\"".format(folderName)
        #     command += "\n"
        #     command += "cmake -G {0} .. -DCMAKE_PREFIX_PATH={1}".format(parm_VSVersion, parm_DCMAKE_PREFIX_PATH)

        # command += '\npause'

        # print(command)
        # p = subprocess.Popen("cmd.exe /c" + "d:/start.bat", stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        # os.system(command)
