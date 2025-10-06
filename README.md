# SHFMod

VERY WIP mod for Silent Hill F. [Dumper-7](https://github.com/Encryqed/Dumper-7) used for generating UE stuff. Using code from [ImGui](https://github.com/ocornut/imgui) and [Universal Dear ImGui Hook](https://github.com/Sh0ckFR/Universal-Dear-ImGui-Hook). [MinHook](https://github.com/TsudaKageyu/minhook) is used for hooking as well. Certain Classes/Structs/etc have been modified. Code is quite messy, will be cleaned later.

The `dxgi.dll` in `loaders` is a simple proxy dll. Will upload source later. It auto injects any file named `SHFMod.dll` into the game on launch.

## Antivirus warnings/blocking/etc
Some antiviruses WILL flag this as malware. It isn't. It gets flagged mostly due to hooking user input for the GUI/controls and due to scanning memory (needed in order to find certain functions/addresses).

## How to Build
1. Clone the repository: `git clone https://github.com/CactusDuper/SHFModPublic.git`
2. Open `SHFMod.sln` in Visual Studio 2022.
3. Set build configuration to **Release** and **x64**.
4. Click **Build -> Build Solution**.
5. The compiled DLL (`SHFMod.dll`) will be located in the `/x64/Release/` folder.

## How to Use
1. Open Steam -> Silent Hill F -> Right click -> Properties -> Installed Files -> Browse -> navigate to `SHf\Binaries\Win64` and place `SHFMod.dll` and `dxgi.dll` (from `loaders` folder) here.
