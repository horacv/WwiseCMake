"""
SDL Complete Setup
Downloads, installs, and configures SDL in one command.
"""

from pathlib import Path
import argparse
import subprocess
import sys

GET_SDL_BASE_SCRIPT = 'get_sdl_base.py'
INSTALL_SDL_BASE_SCRIPT = 'install_sdl_base.py'
GET_SDL_TTF_SCRIPT = 'get_sdl_ttf.py'
INSTALL_SDL_TTF_SCRIPT = 'install_sdl_ttf.py'


def parse_arguments() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(description='SDL Setup')
    parser.add_argument('--platform', const=1, default='windows', type=str, nargs='?', help='')
    parser.add_argument('--delete-installer', const=1, default='True', type=str , nargs='?', help='')
    parser.add_argument('--sdl-version', const=1, default='3.4.12', type=str, nargs='?', help='')
    parser.add_argument('--ttf-version', const=1, default='3.2.2', type=str, nargs='?', help='')
    return parser.parse_args()


def run_command(script_name, args):
    """Run a Python script with arguments and capture the output."""
    python_exe = sys.executable
    script_path = Path(__file__).parent / script_name

    cmd = [python_exe, str(script_path)] + args

    result = subprocess.run(
        cmd,
        capture_output=False,  # Show output in real-time
        text=True
    )

    if result.returncode != 0:
        raise RuntimeError(f"{script_name} failed with exit code {result.returncode}")


def main():
    # Parse arguments
    args = parse_arguments()
    platform = args.platform
    sdl_version = args.sdl_version
    ttf_version = args.ttf_version
    delete_installer = args.delete_installer.lower() == 'true'

    print("\n" + "=" * 60)
    print("Setup SDL & TTF")
    print("=" * 60)
    print(f"Platform: {platform}")
    print(f"Version: {sdl_version}")
    print(f"Delete installer: {'Yes' if delete_installer else 'No'}")
    print("=" * 60)

    try:
        # Download SDL installer
        print("\n[Step 1/4] Downloading sdl installer...")
        print("-" * 60)
        run_command(GET_SDL_BASE_SCRIPT, [platform, sdl_version])

        # Download SDL TTF installer
        print("\n[Step 2/4] Downloading sdl ttf installer...")
        print("-" * 60)
        run_command(GET_SDL_TTF_SCRIPT, [platform, ttf_version])

        # Determine installer filename
        installer_patterns = {
            'mac': {
                'sdl': f'SDL3-{sdl_version}.dmg',
                'ttf': f'SDL3_ttf-{ttf_version}.dmg'
            },
            'windows': {
                'sdl': f'SDL3-devel-{sdl_version}-VC.zip',
                'ttf': f'SDL3_ttf-devel-{ttf_version}-VC.zip'
            },
        }

        # Install and extract API files (SDL)
        print("\n[Step 3/4] Installing SDL files...", end="")

        installer_name = installer_patterns.get(platform).get('sdl')
        if not installer_name:
            raise ValueError(f"Unknown platform: {platform}")

        # Run the installation script with --delete-installer flag if requested
        install_args = [installer_name]
        if delete_installer:
            install_args.append('--delete-installer')

        run_command(INSTALL_SDL_BASE_SCRIPT, install_args)

        # Install and extract API files (SDL_ttf)
        print("\n[Step 4/4] Installing SDL TTF files...", end="")

        installer_name = installer_patterns.get(platform).get('ttf')
        if not installer_name:
            raise ValueError(f"Unknown platform: {platform}")

        # Run the installation script with --delete-installer flag if requested
        install_args = [installer_name]
        if delete_installer:
            install_args.append('--delete-installer')

        run_command(INSTALL_SDL_TTF_SCRIPT, install_args)

        # Success message
        print("\n" + "=" * 60)
        print("✓ SDL Setup Complete!")
        print("=" * 60)

        print("\n" + f"✓ API files installed to: libs/sdl/")
        if delete_installer:
            print(f"✓ Installer cleaned up: {installer_name}")
        else:
            print(f"Installer preserved: {installer_name}")
        print("\nYou can now build your project with SDL")

    except Exception as e:
        print("\n" + "=" * 60)
        print("✗ Setup Failed!")
        print("=" * 60)
        print(f"Error: {e}")
        print("\nTroubleshooting:")
        print("  - Check that the platform and version are correct")
        sys.exit(1)


if __name__ == "__main__":
    main()
