"""
SDL_ttf Installer for CI/CD
Extracts SDL_ttf installer, copies API files to the project, and cleans up.
"""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

# Project directories
PROJECT_ROOT = Path(__file__).parent.parent
SDL_TTF_LIB_DIR = PROJECT_ROOT / "libs" / "sdl"

# Platform-specific configuration
PLATFORM_CONFIG = {
    'windows': {
        'lib_subdir': 'win_x64',
        'anchor_folder': 'lib',
        'api_structure': {
            'inc_dir': 'include/SDL3_ttf',
            'lib_dir': 'lib/x64',
        }
    },
    'mac': {
        'lib_subdir': 'mac',
        'anchor_folder': 'macos-arm64_x86_64',
        'api_structure': {
            'inc_dir': 'macos-arm64_x86_64/SDL3_ttf.framework/Headers',
            'lib_dir': 'macos-arm64_x86_64/SDL3_ttf.framework',
        }
    }
}


def detect_platform(installer_path):
    """Detect platform from installer filename."""
    name = installer_path.name.lower()
    if 'vc' in name and name.endswith('.zip'):
        return 'windows'
    elif name.endswith('.dmg'):
        return 'mac'
    else:
        raise ValueError(f"Cannot detect platform from installer: {installer_path.name}")


def extract_compressed_archive(installer_path, temp_dir):
    """Extract .zip or .tar.gz archive."""
    print("\n" + "Extracting archive...")
    shutil.unpack_archive(str(installer_path), temp_dir)
    print("✓ Extracted archive successfully")


def extract_macos_dmg(installer_path, temp_dir):
    """Extract macOS .dmg installer."""
    print("Extracting macOS DMG...")

    # Mount the DMG
    mount_point = temp_dir / "dmg_mount"
    mount_point.mkdir(exist_ok=True)

    subprocess.run(
        ['hdiutil', 'attach', str(installer_path), '-mountpoint', str(mount_point), '-nobrowse'],
        check=True
    )

    try:
        # Copy contents
        for item in mount_point.iterdir():
            if item.is_dir():
                shutil.copytree(item, temp_dir / item.name, dirs_exist_ok=True)
            else:
                shutil.copy2(item, temp_dir / item.name)
    finally:
        # Unmount
        subprocess.run(['hdiutil', 'detach', str(mount_point)], check=False)

    print("✓ Extracted DMG")


def copy_api_files(temp_dir, platform):
    """Copy SDL_ttf files from the temp directory to the project."""
    print("\nCopying SDL_ttf files to project...")

    config = PLATFORM_CONFIG[platform]
    api_structure = config['api_structure']

    # Find the actual API directory in temp (may be nested)
    api_dirs = list(temp_dir.rglob(config['anchor_folder']))
    if not api_dirs:
        # Try alternate structure
        api_dirs = [temp_dir]

    source_root = api_dirs[0].parent if api_dirs else temp_dir

    # Copy SDL_ttf headers
    src = source_root / api_structure['inc_dir']
    dst = SDL_TTF_LIB_DIR / "include" / "SDL3_ttf"
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        print(f"✓ Copied core headers ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: Core headers not found at {src}")

    # Copy core libraries
    src: Path = source_root / api_structure['lib_dir']
    dst: Path = SDL_TTF_LIB_DIR / "lib" / str(config['lib_subdir'])
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        if src.exists():
            if platform == "mac":
                # Copy the whole framework bundle into libs/sdl/lib/mac/SDL3_ttf.framework
                dst = SDL_TTF_LIB_DIR / "lib" / str(config['lib_subdir']) / src.name
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copytree(src, dst, dirs_exist_ok=True)
                print(f"✓ Copied core framework ({src.name})")
            else:
                dst = SDL_TTF_LIB_DIR / "lib" / str(config['lib_subdir'])
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copytree(src, dst, dirs_exist_ok=True)
                print(f"✓ Copied core libraries ({len(list(dst.glob('*')))} files)")
        else:
            print(f"⚠ Warning: Core libraries not found at {src}")


def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: python install_sdl_ttf.py <installer_file> [--delete-installer]")
        print("Example: python install_sdl_ttf.py SDL3_ttf-devel-3.2.2-VC.zip")
        print("Example: python install_sdl_ttf.py SDL3_ttf-devel-3.2.2-VC.zip --delete-installer")
        print("\nOptions:")
        print("  --delete-installer    Delete the installer file after successful installation")
        sys.exit(1)

    installer_path = Path(sys.argv[1])
    delete_installer = len(sys.argv) == 3 and sys.argv[2] == '--delete-installer'

    # Validate installer exists
    if not installer_path.is_file():
        print(f"Error: Installer not found: {installer_path}")
        sys.exit(1)

    print("\n" + "=" * 60)
    print(f"SDL_ttf Installer", end="")
    print("\n" + "=" * 60)

    # Detect platform
    try:
        platform = detect_platform(installer_path)
        print("\n" + f"Installer found: {installer_path.name}")
        print(f"Platform: {platform}")
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)

    # Create a temporary directory
    temp_dir = Path(tempfile.mkdtemp(prefix="temp_sdl_install_"))
    print(f"Temp directory: {temp_dir}")

    try:
        # Extract installer
        if platform == 'windows':
            extract_compressed_archive(installer_path, temp_dir)
        elif platform == 'mac':
            extract_macos_dmg(installer_path, temp_dir)

        # Copy API files to the project
        copy_api_files(temp_dir, platform)

        print("\n✓ Installation complete!")
        print(f"SDL_ttf files copied to: {SDL_TTF_LIB_DIR}")

        # Delete installer if requested
        if delete_installer:
            print("\n[--delete-installer] flag passed. Deleting installer...")
            installer_path.unlink()
            print(f"✓ Deleted {installer_path.name}")

    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)
    finally:
        # Cleanup temporary directory
        print("\nCleaning up...")
        shutil.rmtree(temp_dir, ignore_errors=False)
        print("✓ Temporary files removed")


if __name__ == "__main__":
    main()