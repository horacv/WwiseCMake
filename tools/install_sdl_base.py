"""
SDL Installer for CI/CD
Extracts SDL installer, copies API files to the project, and cleans up.
"""

import sys
import subprocess
import shutil
import tempfile
from pathlib import Path

# Project directories
PROJECT_ROOT = Path(__file__).parent.parent
SDL_LIB_DIR = PROJECT_ROOT / "libs" / "sdl"

# Platform-specific configuration
PLATFORM_CONFIG = {
    'windows': {
        'lib_subdir': 'win_x64',
        'api_structure': {
            'inc_dir': 'include/SDL3',
            'lib_dir': 'lib/x64',
        }
    },
    'mac': {
        'lib_subdir': 'mac',
        'api_structure': {
            'inc_dir': 'include/SDL3',
            'lib_dir': 'lib/x64',
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
    print("Extracting archive...")
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
    """Copy SDL files from temp directory to project."""
    print("\nCopying SDL files to project...")

    config = PLATFORM_CONFIG[platform]
    api_structure = config['api_structure']

    # Find the actual API directory in temp (may be nested)
    api_dirs = list(temp_dir.rglob('lib'))
    if not api_dirs:
        # Try alternate structure
        api_dirs = [temp_dir]

    source_root = api_dirs[0].parent if api_dirs else temp_dir

    # Copy SDL headers
    src = source_root / api_structure['inc_dir']
    dst = SDL_LIB_DIR / api_structure['inc_dir']
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        print(f"✓ Copied core headers ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: Core headers not found at {src}")

    # Copy core libraries
    src = source_root / api_structure['lib_dir']
    dst = SDL_LIB_DIR / "lib" / config['lib_subdir']
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        print(f"✓ Copied core libraries ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: Core libraries not found at {src}")


def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: python install_sdl_base.py <installer_file> [--delete-installer]")
        print("Example: python install_sdl_base.py SDL3-devel-3.2.24-VC.zip")
        print("Example: python install_sdl_base.py SDL3-devel-3.2.24-VC.zip --delete-installer")
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
    print(f"SDL Installer")
    print("\n" + "=" * 60)

    # Detect platform
    try:
        platform = detect_platform(installer_path)
        print(f"Installer found: {installer_path.name}")
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
        print(f"SDL files copied to: {SDL_LIB_DIR}")

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