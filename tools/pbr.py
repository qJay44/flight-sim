import os
import zipfile
import argparse
import subprocess


def extract(zipPath: str, extractPath: str) -> dict[str, str]:
    nameAliases = {
        "albedo":    ["albedo", "base_color"],
        "normal":    ["normal"],
        "ao":        ["ao", "ambient", "ambient_occlusion"],
        "roughness": ["roughness"],
        "metallic":  ["metallic"],
        "height":    ["height"],
    }

    def isAlias(filename: str, alias: str) -> bool:
        f = filename.lower()
        for m in nameAliases[alias]:
            if f.find(m) != -1:
                return True
        return False

    res = {
        "albedo": "",
        "normal": "",
        "ao": "",
        "roughness": "",
        "metallic": "",
        "height": "",
    }

    with zipfile.ZipFile(zipPath, 'r') as zipRef:
        for entry in zipRef.infolist():
            if entry.is_dir():
                continue

            filename = os.path.basename(entry.filename)

            if not filename:
                continue

            targetPath = ""

            for alias in nameAliases.keys():
                if isAlias(filename, alias):
                    print(f"Found [{alias:<9}]: [{filename}]")

                    filename = f"{alias}.{filename.split('.')[1]}"
                    targetPath = os.path.join(extractPath, filename)
                    res[alias] = targetPath
                    break;

            if (len(targetPath) == 0):
                print(f"\033[33mSkipping [{filename}]\033[0m")
                continue

            with zipRef.open(entry) as source, open(targetPath, "wb") as target:
                target.write(source.read())

    print()

    return res


def downsample(images: dict[str, str], size: int) -> None:
    for img in images.values():
        subprocess.run(["magick", img, "-resize", f"{size}x{size}\\>", img])


def combine(images: dict[str, str], extractPath: str) -> None:
    def getInfo(name: str) -> tuple[int, int, str]:
        cmdOutput = subprocess.run(["identify", "-format", r"%[width]x%[height],%[channels]", images[name]], capture_output=True, text=True)
        sizeStr, channelStr = cmdOutput.stdout.split(',')
        sizex, sizey = [int(s) for s in sizeStr.split('x')]
        return (sizex, sizey, channelStr)

    targetPath = os.path.join(extractPath, "ORMH.png")
    sizeGeneralX = sizeGeneralY = -1
    nameCommands = {
        "ao":        ["-channel", "R", "-compose", "CopyRed"],
        "roughness": ["-channel", "G", "-compose", "CopyGreen"],
        "metallic":  ["-channel", "B", "-compose", "CopyBlue"],
        "height":    ["-channel", "A", "-compose", "CopyAlpha", "-colorspace", "sRGB"],
    }

    for name, path in images.items():
        if (len(path) == 0 or name not in nameCommands):
            continue

        sizeX, sizeY, channelStr = getInfo(name)

        if sizeX != sizeY:
            print(f"Width [{sizeX}] != Height [{sizeY}]")
            exit(1)

        if sizeGeneralX == -1:
            sizeGeneralX = sizeX
            sizeGeneralY = sizeY
            subprocess.run(["magick", "-size", f"{sizeX}x{sizeY}", "canvas:black", targetPath])
        elif sizeX != sizeGeneralX or sizeY != sizeGeneralY:
            print(f"[{name}] has unexpected size [{sizeX}x{sizeY}] vs [{sizeGeneralX}x{sizeGeneralY}]")
            exit(1)

        print(f"Combine source [{name:<9}]: [{sizeX}x{sizeY}, {channelStr}]")

        subprocess.run(["magick", targetPath, path, *nameCommands[name], "-composite", targetPath])

    print()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=\
    """
    Exctract and combine PBR textures. Make sure only .zip files inside [path0] when using -a"

    Examples when located in the project dir, and there are multiple .zip files in the "~/Downloads/pbrs" folder:

    python tools/pbr-pack.py ~/Downloads/pbrs/graniterockface1-Unreal-Engine.zip res/tex/pbr -v
    python tools/pbr.py ~/Downloads/pbrs res/tex/pbr -a
    python tools/pbr.py ~/Downloads/pbrs res/tex/pbr -d 256 -a

    """)

    parser.add_argument("path0", help="zip file path (folder with -a)")
    parser.add_argument("path1", help="exctract folder path")
    parser.add_argument("-d", "--downsample", type=int, default=0)
    parser.add_argument("-a", "--all", action="store_true")

    args = parser.parse_args()


    if (args.all):
        zipFolder = args.path0
        for entry in os.scandir(zipFolder):
            if entry.is_file():
                parentDir = os.path.basename(entry).split('.')[0]
                extractPath = os.path.join(args.path1, parentDir)
                os.makedirs(extractPath, exist_ok=True)

                mappedImages = extract(entry.path, extractPath)
                if (args.downsample):
                    downsample(mappedImages, args.downsample)

                combine(mappedImages, extractPath)
    else:
        zipPath = args.path0
        parentDir = os.path.basename(zipPath).split('.')[0]
        extractPath = os.path.join(args.path1, parentDir)
        os.makedirs(extractPath, exist_ok=True)

        mappedImages = extract(zipPath, extractPath)

        if (args.downsample):
            downsample(mappedImages, args.downsample)

        combine(mappedImages, extractPath)

