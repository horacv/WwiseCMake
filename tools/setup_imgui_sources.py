"""
imgui Downloader
Downloads the imgui libraries for the specified platform.
"""

from pathlib import Path
import requests
import shutil
import sys
import tempfile

IMGUI_RELEASES_BASE_URL = 'https://github.com/ocornut/imgui/archive/refs/tags'
IMGUI_PROJ_SRC_DIR = Path(__file__).parent.parent / "libs" / "imgui"

file_template = 'v{version}.tar.gz'


def download_with_progress(url, filepath, chunk_size=65536):
    """Download a file with a progress indication."""
    response = requests.get(url, stream=True, allow_redirects=True)
    response.raise_for_status()

    total_size = int(response.headers.get('content-length', 0))
    downloaded = 0

    with open(filepath, 'wb') as f:
        for chunk in response.iter_content(chunk_size=chunk_size):
            if chunk:
                f.write(chunk)
                downloaded += len(chunk)
                if total_size:
                    progress = (downloaded / total_size) * 100
                    print(f"\rDownloading: {progress:.1f}%", end='', flush=True)

    # New line after progress
    print()
    return downloaded


def extract_installer(installer_path, temp_directory):
    """Extract .zip or .tar.gz archive."""
    print("\n" + "Extracting archive...")
    shutil.unpack_archive(str(installer_path), temp_directory)
    print("✓ Extracted archive successfully")


def copy_api_files(temp_dir):
    """Copy imgui libs from the temp directory to the project."""
    print("\nCopying imgui files to project...")

    src_temp_dirs = list(temp_dir.rglob('backends'))

    # Copy headers
    src: Path = src_temp_dirs[0] if src_temp_dirs else Path()
    dst: Path = IMGUI_PROJ_SRC_DIR
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src.parent, dst, dirs_exist_ok=True)
        print(f"✓ Copied imgui sources ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: imgui sources not found at {src.parent}")


def main():
    # Validate arguments
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: python setup_imgui_sources.py <imgui_version> [--delete-installer]")
        print("Example: python setup_imgui_sources.py 1.92.6")
        print("Example: python setup_imgui_sources.py 1.92.6 --delete-installer")
        print("\nOptions:")
        print("  --delete-installer    Delete the installer file after successful installation")
        sys.exit(1)

    imgui_version = sys.argv[1]
    delete_installer = len(sys.argv) == 3 and sys.argv[2] == '--delete-installer'
    filename = file_template.format(version=imgui_version)
    download_url = f"{IMGUI_RELEASES_BASE_URL}/" f"{filename}"

    try:
        print()
        print("\n" + "=" * 60)
        print(f"Setup ImGui Sources")
        print("=" * 60)

        # Validate download link
        print("\n" + "Validating download link...")
        link_response = requests.head(
            download_url,
            timeout=10
        )
        link_response.raise_for_status()
        print("✓ Link validated")

        # Step 2: Download the file
        print("\n" + f"Downloading {filename}...", end="")
        file_size = download_with_progress(download_url, filename)
        print(f"✓ Download complete: {filename} ({file_size:,} bytes)")

    except requests.exceptions.HTTPError as e:
        print(f"HTTP Error: {e}")
        print(f"Response: {e.response.text if e.response else 'No response'}")
        sys.exit(1)
    except requests.exceptions.RequestException as e:
        print(f"Network Error: {e}")
        sys.exit(1)
    except KeyError as e:
        print(f"Error parsing API response: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)

    installer_path = Path(filename)

    # Validate installer exists
    if not installer_path.is_file():
        print(f"Error: Installer not found: {installer_path}")
        sys.exit(1)

    # Create a temporary directory
    temp_dir = Path(tempfile.mkdtemp(prefix="temp_imgui_install_"))
    print(f"✓ Temp directory: {temp_dir}")

    try:
        # Extract installer
        extract_installer(installer_path, temp_dir)

        # Copy API files to the project
        copy_api_files(temp_dir)

        print("\n✓ Installation complete!")
        print(f"imgui files copied to: {IMGUI_PROJ_SRC_DIR}")

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