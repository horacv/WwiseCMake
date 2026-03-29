### Tools Directory

This directory contains Python scripts to automate the downloading and installation of required libraries (SDL, ImGui) for the project.

#### Prerequisites

- **Python 3.x**
- **Requests library**: Install via `pip install requests`

---

### ImGui Scripts

#### `setup_imgui_sources.py`
Downloads and installs ImGui source code.

```bash
python setup_imgui_sources.py <imgui_version> [--delete-installer]
```
- **imgui_version**: e.g., `1.92.6`.
- **--delete-installer**: (Optional) Deletes the downloaded archive after successful extraction.

**Example:**
```bash
python setup_imgui_sources.py 1.92.6 --delete-installer
```

---

### SDL Scripts

#### `setup_sdl_and_ttf.py` (Recommended)
Downloads and installs both SDL and SDL_ttf in one step.

```bash
python setup_sdl_and_ttf.py --platform <platform> --sdl-version <sdl_version> --ttf-version <ttf_version> [--delete-installer <True|False>]
```
- **--platform**: `windows` (default) or `mac`.
- **--sdl-version**: e.g., `3.4.2` (default).
- **--ttf-version**: e.g., `3.2.2` (default).
- **--delete-installer**: (Optional) `True` (default) or `False`. Deletes the downloaded archives after successful extraction.

**Example:**
```bash
python setup_sdl_and_ttf.py --platform windows --sdl-version 3.4.2 --ttf-version 3.2.2
```

#### `get_sdl_base.py` / `get_sdl_ttf.py`
Downloads the SDL or SDL_ttf archive only.
```bash
python get_sdl_base.py <platform> <sdl_version>
python get_sdl_ttf.py <platform> <ttf_version>
```

#### `install_sdl_base.py` / `install_sdl_ttf.py`
Extracts and copies SDL or SDL_ttf files from a local archive to the project's `libs/sdl` directory.
```bash
python install_sdl_base.py <installer_file> [--delete-installer]
python install_sdl_ttf.py <installer_file> [--delete-installer]
```

**Example:**
```bash
python install_sdl_base.py SDL3-devel-3.4.2-VC.zip --delete-installer
python install_sdl_ttf.py SDL3_ttf-devel-3.2.2-VC.zip --delete-installer
```

---

### Wwise Scripts

#### `install_wwise_sdk.py`
Copies Wwise SDK files from a local installation to the project's `libs/wwise` directory.

```bash
python install_wwise_sdk.py <sdk_path> <platform>
```
- **sdk_path**: Path to the Wwise SDK folder (e.g., `C:\Audiokinetic\Wwise2024.1.12.9034\SDK`).
- **platform**: `windows` or `mac`.

**Example:**
```bash
python install_wwise_sdk.py "C:\Audiokinetic\Wwise2024.1.12.9034\SDK" windows
```
