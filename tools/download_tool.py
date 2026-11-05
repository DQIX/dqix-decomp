import argparse
from pathlib import Path
from get_platform import get_platform
import zipfile
import tarfile
import hashlib
import io
import requests
import stat

root_path = Path(__file__).parent.parent


platform = get_platform()
if platform is None:
    exit(1)

parser = argparse.ArgumentParser()
parser.add_argument("tool")
parser.add_argument("tag")
parser.add_argument("--path", type=Path, required=True)
args = parser.parse_args()


def dsd_url(tag: str) -> str:
    return f'https://github.com/AetiasHax/ds-decomp/releases/download/{tag}/dsd-{platform.system}-{platform.machine}{platform.exe}'

def mwccarm_url(tag: str) -> str:
    return 'http://decomp.aetias.com/files/mwccarm.zip'

def wibo_url(tag: str) -> str:
    if tag.startswith('sha256:'):
        return f'https://ghcr.io/v2/decompals/wibo/blobs/{tag}'
    return f'https://github.com/decompals/wibo/releases/download/{tag}/wibo'

def objdiff_url(tag: str) -> str:
    return f'https://github.com/encounter/objdiff/releases/download/{tag}/objdiff-cli-{platform.system}-{platform.machine}{platform.exe}'

TOOLS = {
    "dsd": dsd_url,
    "mwccarm": mwccarm_url,
    "wibo": wibo_url,
    "objdiff": objdiff_url,
}


download_url = TOOLS[args.tool](args.tag)
print(f'\nDownloading {args.tool} {args.tag}...')
from_image = download_url.startswith('https://ghcr.io/')
headers = {}
if from_image:
    token = requests.get('https://ghcr.io/token?scope=repository:decompals/wibo:pull').json()['token']
    headers['Authorization'] = f'Bearer {token}'
response = requests.get(download_url, headers=headers)
content = response.content
if from_image:
    if 'sha256:' + hashlib.sha256(content).hexdigest() != args.tag:
        exit(f'{args.tool} {args.tag}: digest mismatch')
    content = tarfile.open(fileobj=io.BytesIO(content)).extractfile('usr/local/bin/wibo').read()
if download_url.endswith('.zip'):
    zip_file = zipfile.ZipFile(io.BytesIO(content))
    zip_file.extractall(args.path)
else:
    out_path: Path = args.path
    with out_path.open('wb') as f:
        f.write(content)
    out_path.chmod(out_path.stat().st_mode | stat.S_IEXEC)
