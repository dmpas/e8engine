#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  setversion.py
#  

import sys
from subprocess import check_output

product_version = '0.3'
release_version = '1'
version_dot = '0.3.1'
version_comma = '0,3,1'
build_number = '1'
try:
	revision_id = check_output(['hg', 'id', '--id']).strip()
except:
	revision_id = '0'

def get_content(path):
	with open(path, 'r') as content_file:
		content = content_file.read()
	return content
	
def set_content(path, content):
	with open(path, 'w') as output:
		output.write(content)
	
def update_file(path_in, path_out):
	content = get_content(path_in)
	content = content.replace(
		'$build_number', build_number).replace(
		'$product_version', product_version).replace(
		'$version_dot', version_dot).replace(
		'$version_comma', version_comma).replace(
		'$revision', revision_id)
		
	set_content(path_out, content)
		
	

def main():
	
	global build_number
	global revision_id
	
	if len(sys.argv) > 1:
		build_number = sys.argv[1]
		
	update_file('VersionInfo.tpl', 'VersionInfo.rc')
	update_file('version.tpl', 'version.h')
	
	update_file('plugins/e8std/VersionInfo.tpl', 'plugins/e8std/VersionInfo.rc')
	update_file('plugins/files/VersionInfo.tpl', 'plugins/files/VersionInfo.rc')
	update_file('e8core/utf/VersionInfo.tpl', 'e8core/utf/VersionInfo.rc')
	update_file('e8core/variant/VersionInfo.tpl', 'e8core/variant/VersionInfo.rc')
	
	return 0

if __name__ == '__main__':
	main()

