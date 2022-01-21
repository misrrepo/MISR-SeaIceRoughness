#!/usr/bin/python3.6

import MisrToolkit as mtk
print('hi!')

latList = [84.741173, 80.13375,80.132784]
lonList = [230.999962, 272.293766, 272.300945]

for i in range(3):
	print(i)
	try:
		ret=mtk.latlon_to_bls(97, 275, latList[i], lonList[i])
	except (Exception):
		continue
	print('hi to %d!' %i)