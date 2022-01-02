#!/usr/bin/python3

from getpass import getpass
from html.parser import HTMLParser
from pathlib import Path
from typing import Any, List

from requests import Session
import requests
from requests import Response


# This class is used to process 
class MyHTMLParser(HTMLParser):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.hrefs: List[str] = []

    def handle_starttag(self, tag, attrs):
        if tag == "a":
            for (attr, value) in attrs:
                if attr == "href":
                    self.hrefs.append(value)

    def get_hrefs(self, response: Response) -> List[str]:
        self.feed(str(response._content))
        return self.hrefs

def get_session() -> Session:
    # This code will prompt you to enter your username and password
    username = input("Earthdata username: ")
    password = getpass("Earthdata password: ")

    # create a session that can be used to log in
    session = Session()
    session.auth = (username, password)
    
    return session

def is_correct_ext(file: str, filters: List[str]):
    for filter in filters:
        if file.lower().endswith(filter.lower()):
            return True
    return False

def get_files_to_download(session: Session, filter_list: List[str]) -> List[str]:

    # this function returns true if a href is a child ref
    is_child_href = lambda href: not href.startswith('http') and not (href.startswith('/') or href.startswith('#') or href.startswith('mailto'))

    # walk the directory to find all directory and file URLs
    print("Getting download files.")
    urls = [starting_url]
    files = []
    for url in urls:

        # get the page's contents
        print(f"Checking {url}")
        _redirect = session.get(url)
        _response = session.get(_redirect.url)

        # this class populates the hrefs list
        parser = MyHTMLParser()
        hrefs = parser.get_hrefs(_response)
        for href in hrefs:
            new_url = url+href
            if is_child_href(href) and href != url:
                if href.endswith('/'):
                    urls.append(new_url)
                elif is_correct_ext(new_url, filter_list) and new_url not in files:
                    files.append(new_url)
    return files

def verify_download(session: Session, files: List[str], output_dir: Path) -> List[str]:
    # create a data directory in the current working directory to store the downloaded files
    output_dir.mkdir(exist_ok=True)

    # should existing files be overwritten
    print("Checking for existing files")
    local_files = [file.name.lower() for file in output_dir.iterdir()]
    existing_files = list(filter(lambda file: Path(file).name.lower() in local_files, files))
    if existing_files and input(f"Overwrite {len(existing_files)} exiting files? [y/n]: ") not in ('y', 'ye', 'yes'):
        files = list(filter(lambda file: file not in existing_files, files))

    print('found files: %s' %len(files))
    print('first file on list:')
    print(files[0])

    # calculate total file size so that the user can verify they have enough space
    print("Getting download size.")
    total_size = 0
    if not files:
        print("No files matched the filter or no files were found in the directory, exiting")
        exit()

    for iterNum, file in enumerate(files):
        _redirect = session.head(file)
        _response = session.head(_redirect.url)
        print('checking size for file: %d/%d' %(iterNum+1, len(files)))
        total_size += int(_response.headers.get('content-length'))

    # inform the user before starting download
    if input(f"Download {len(files)}, {total_size // 1024**2} MB? [y/n]: ").lower() not in ('y', 'ye', 'yes'):
        print("Exiting, consider adding more filters or starting at a lower level folder")
        exit()

    return files

def download(session: Session, files: List[str], output_dir: Path) -> None:
    # The following code downloads the files
    print(f"Downloading {len(files)} files.")
    for i, file in enumerate(files):
        print(f"Downloading file {i+1} of {len(files)}", end="\r")
        file_path = output_dir.joinpath(file.split('/')[-1])
        with session.get(file) as _redirect:
            _redirect = session.get(file) 
            _response = session.get(_redirect.url) 
            with file_path.open('wb') as file:
                file.write(_response._content)

if __name__ == "__main__":
    # This URL is the starting directory
    starting_url = input("Enter the top level URL: ")
    # enter "test" for the url to download form a small, 20MB directory.
    if starting_url == "test":
        starting_url = "https://asdc.larc.nasa.gov/data/MISR/MI3DCLDN.002/2000.09.20/"
        
    # the otuput directory Path('data') will save it to a file called 'data' in the current working directory
    output_dir = Path('data')

    filter_text = input("File types to download. (Ex: .ict, .h5, blank for all): ")
    filter_list = [filter.strip() for filter in filter_text.split(',')]

    session = get_session()
    files = get_files_to_download(session, filter_list)
    files = verify_download(session, files, output_dir)
    download(session, files, output_dir)