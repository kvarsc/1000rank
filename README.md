# 1000rank

1000rank is a novel ranking algorithm for Super Smash Bros. Ultimate competitors, powered by [smashdata.gg](https://smashdata.gg/).

Note that this repository was made to be compiled and run on Windows (64-bit). It may be possible to compile the code on other operating systems, but I've never done it.

## Prerequisites

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs). Be sure to include the "Desktop development with C++" workload during installation.
2. Install [vcpkg](https://github.com/microsoft/vcpkg). 1000rank's Visual Studio project file will expect this to be in `C:\vcpkg` by default.
3. Install [cpr](https://github.com/libcpr/cpr): `.\vcpkg install cpr:x64-windows`.
4. Install [mstch](https://github.com/no1msd/mstch): `.\vcpkg install mstch:x64-windows`.
5. Install [lexbor](https://github.com/lexbor/lexbor). Since `lexbor`'s README assumes you're on a UNIX system, here's a quick guide to installing `lexbor` on Windows 11:
    - Clone the `lexbor` git repo: `git clone https://github.com/lexbor/lexbor.git`. 1000rank's Visual Studio project file will expect this to be in `C:\lexbor` by default.
    - Open "x64 Native Tools Command Prompt for VS 2022" (you should be able to find this by typing it into your Start menu if you have VS 2022 installed).
    - Navigate to the `lexbor` directory, e.g. `cd C:\lexbor`.
    - Build `lexbor` with CMake: `cmake . -G "Visual Studio 17 2022" -A x64 -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_BUILD_EXAMPLES=ON`.
    - Run the install command: `cmake --build . --config Release --target install`.

## Quick Start

1. Open Visual Studio.
2. Select "Clone a Repository" from the main menu.
3. Clone `https://github.com/kvarsc/1000rank.git`.
4. Build the application in Visual Studio.
5. Copy the file `lexbor.dll` from the `Release` folder of your `lexbor` installation to the directory containing the built `1000rank` application (`1000rank\x64\Release` or `1000rank\x64\Debug`).
6. If you're using the default `config.json` file that comes with the repo, head to https://1000rank.com and download the HTML files of the [2019 rankings](https://1000rank.com/2019rankings.html) and the [2022 rankings](https://1000rank.com/2022rankings.html) to the `1000rank\1000rank` directory. Make sure you download these files as HTML only.
7. Run the application in Visual Studio.

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.