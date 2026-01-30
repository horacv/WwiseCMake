### Tools Directory

This directory contains Python scripts to automate the downloading and installation of required libraries (SDL) for the project.

#### Prerequisites

- **Python 3.x**
- **Requests library**: Install via `pip install requests`

---

### SDL Scripts

#### `setup_sdl_and_ttf.py` (Recommended)
Downloads and installs both SDL and SDL_ttf in one step.

```bash
python setup_sdl_and_ttf.py --platform <platform> --sdl-version <sdl_version> --ttf-version <ttf_version> [--delete-installer <True|False>]
```
- **--platform**: `windows` (default) or `mac`.
- **--sdl-version**: e.g., `3.2.24` (default).
- **--ttf-version**: e.g., `3.2.2` (default).
- **--delete-installer**: (Optional) `True` (default) or `False`. Deletes the downloaded archives after successful extraction.

**Example:**
```bash
python setup_sdl_and_ttf.py --platform windows --sdl-version 3.2.24 --ttf-version 3.2.2
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
