"""
SDL Downloader
Downloads the SDL libraries for the specified platform.
"""

import requests
import sys

sdl_releases_base_url = 'https://github.com/libsdl-org/SDL/releases/download/release-{version}'

# Platform configuration
PLATFORM_CONFIG = {
    'mac': {
        'filename': 'SDL3-{version}.dmg',
    },
    'windows': {
        'filename': 'SDL3-devel-{version}-VC.zip',
    }
}

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

    print()  # New line after progress
    return downloaded


def main():
    # Validate arguments
    if len(sys.argv) != 3:
        print("Usage: python get_sdl_base.py <platform> <sdl_version>")
        print("Platforms: mac, windows")
        print("Example: python get_sdl_base.py windows 3.4.2")
        sys.exit(1)

    platform = sys.argv[1]
    sdl_version = sys.argv[2]

    # Validate platform
    if platform not in PLATFORM_CONFIG:
        print(f"Error: Unsupported platform '{platform}'")
        print(f"Supported platforms: {', '.join(PLATFORM_CONFIG.keys())}")
        sys.exit(1)

    # Get platform configuration
    config = PLATFORM_CONFIG[platform]
    base_path = sdl_releases_base_url.format(version=sdl_version)
    filename = config['filename'].format(version=sdl_version)

    # Build download URL
    download_url = f"{base_path}/" f"{filename}"

    try:
        # Step 1: Validate download link
        print("Validating download link...")
        link_response = requests.head(
            download_url,
            timeout=10
        )
        link_response.raise_for_status()

        print("✓ Link validated")

        # Step 2: Download the file
        print(f"Downloading {filename}...")
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


if __name__ == "__main__":
    main()