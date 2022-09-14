#!/usr/bin/python3

'''
how this code works:  setup server path and download path in this code and then run it 
also set these directory paths: 

starting_url
home_dir
out_dir_label

'''


from getpass import getpass
from html.parser import HTMLParser
from pathlib import Path
from typing import Any, List

import os, sys
from requests import Session 
import requests
from requests import Response


print('%s' %sys.version)


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
    # username = input("Earthdata username: ")
    # password = getpass("Earthdata password: ")

    username = "emosadegh@nevada.unr.edu"
    password = "E@reno2021"

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

    # this function returns true if a href is a child ref == menaing being hdf file on remote
    is_child_href = lambda href: not href.startswith('http') and not (href.startswith('/') or href.startswith('#') or href.startswith('mailto'))

    # walk the directory to find all directory and file URLs
    print("Getting download files.")
    urls = [starting_url]
    files = []  # remote files
    for url in urls:
        # get the page's contents
        print(f"Checking {url}")
        _redirect = session.get(url)
        _response = session.get(_redirect.url)

        # this class populates the hrefs list
        parser = MyHTMLParser()
        hrefs = parser.get_hrefs(_response)
        for href in hrefs:  # href is each file on remote
            new_url = url+href
            if is_child_href(href) and href != url:  # if be a unique hdf files
                if href.endswith('/'):
                    urls.append(new_url)
                elif is_correct_ext(new_url, filter_list) and new_url not in files:
                    files.append(new_url)
    print('num. of remote files: %s' %len(files))
    return files


def verify_download(session: Session, files: List[str], output_dir: Path) -> List[str]:
    # create a data directory in the current working directory to store the downloaded files
    output_dir.mkdir(exist_ok=True)

    # should existing files be overwritten
    print("Checking for existing files")
    local_files = [file.name.lower() for file in output_dir.iterdir()]
    print('num. of local files: %s' %(len(local_files)))

    # turn off because of error
    # existing_files = list(filter(lambda file: Path(file).name.lower() in local_files, files))  # file==variable; filter(function/condition that returns true(==where ture, extract from iterable), iterable)
    # print('len existing files: %s' %len(existing_files))

    # overwrite_existing_files = False
    # if existing_files and input(f"Overwrite {len(existing_files)} exiting files? [y/n]: ") not in ('y', 'ye', 'yes'):
    #     files = list(filter(lambda file: file not in existing_files, files))  # remote files that not exist on local storage

    if (len(local_files)!=0):
        print('excluding exisitng files on local storage from download')
        existing_files = list(filter(lambda file: Path(file).name.lower() in local_files, files))  # file==variable; filter(function/condition that returns true(==where ture, extract from iterable), iterable)
        print('len existing files in local storage: %s' %len(existing_files))

        overwrite_existing_files = False  # later move to f(argument)

        if (overwrite_existing_files == False):
            files = list(filter(lambda file: file not in existing_files, files))  # remote files that not exist on local storage
            print('num. of remote files we will download w/o overwriting: %s' %len(files))


    # print(type(files))
    print('remote files: %s' %len(files))
    print('first file on list:')
    print(files[0])
    print('***************************************************')

    # calculate total file size so that the user can verify they have enough space
    # print("Getting download size.")
    # # total_size = 0
    # if not files:
    #     print("No files matched the filter or no files were found in the directory, exiting")
    #     exit()
    #
    # for iterNum, file in enumerate(files):
    #     # print('remote file: %s' %file)
    #     # print(type(file))
    #     main_file = file.split('/')[-1]
    #     # print('file to download: %s' %main_file)
    #     starting_label = main_file.split('_')[0]
    #     # print('starting label: %s' %starting_label)
    #     if (starting_label !='MISR'):
    #         # print('starting label is wrong; continue')
    #         continue
    #     print(file)
    #     _redirect = session.head(file)
    #     _response = session.head(_redirect.url)
    #     # print('checking size for file: %d/%d' %(iterNum+1, len(files)))
        # total_size += int(_response.headers.get('content-length'))
    #
    # inform the user before starting download
    # if input(f"Download {len(files)}, {total_size // 1024**2} MB? [y/n]: ").lower() not in ('y', 'ye', 'yes'):
    #     print("Exiting, consider adding more filters or starting at a lower level folder")
    #     exit()

    return files


def download(session: Session, files: List[str], output_dir: Path) -> None:
    # The following code downloads the files
    print(f"Downloading {len(files)} files.")
    for i, file in enumerate(files):
        print("Downloading file %s of %s" %(i+1,len(files))) #, end="\r")
        file_path = output_dir.joinpath(file.split('/')[-1])
        print('file path: %s' %file_path)
        with session.get(file) as _redirect:
            _redirect = session.get(file) 
            _response = session.get(_redirect.url) 
            with file_path.open('wb') as file:
                file.write(_response._content)



####################################################################################
if __name__ == "__main__":
    # This URL is the starting directory
    # starting_url = input("Enter the top level URL: ")
    starting_url = "https://xfr139.larc.nasa.gov/MISR/Subset_Products/202209138488/"


    # path to download directory on local machine
    home_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks"


    out_dir_label = "july_2016_rccm_9cams" # code will make this directory if not available on disk

    ################################################################################
    # old code
    # enter "test" for the url to download form a small, 20MB directory.
    # if starting_url == "test":
    #     starting_url = "https://xfr139.larc.nasa.gov/MISR/Subset_Products/202203261938/"
        
    # the output directory Path('data') will save it to a file called 'data' in the current working directory
    # output_dir = Path('data')

    out_dir_fp = os.path.join(home_dir, out_dir_label)
    output_dir = Path(out_dir_fp)
    print('output directory:')
    print(output_dir)

    # filter_text = input("File types to download. (Ex: .ict, .h5, blank for all): ")
    filter_text = ".hdf"
    filter_list = [filter.strip() for filter in filter_text.split(',')]
    print(filter_list)

    session = get_session()
    files = get_files_to_download(session, filter_list)
    files = verify_download(session, files, output_dir)
    download(session, files, output_dir)

