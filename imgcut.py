#!/usr/bin/env python
# -*- coding: utf-8 -*-

from PIL import Image, ImageDraw
import math

def cut1(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	img = Image.open(ln[0])
	img = img.resize((120, 120), Image.ANTIALIAS)
	image.paste(img, (6, 6))
	return image
	
def cut2(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	img = Image.open(ln[0])
	img = img.resize((60, 60), Image.ANTIALIAS)
	image.paste(img, (4, 36))
	img = Image.open(ln[1])
	img = img.resize((60, 60), Image.ANTIALIAS)
	image.paste(img, (68, 36))
	return image
	
def cut3(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =4  
	img = Image.open(ln[0])
	img = img.resize((60, 60), Image.ANTIALIAS)
	image.paste(img, (int(132/2-60/2), _y))
	img = Image.open(ln[1])
	img = img.resize((60, 60), Image.ANTIALIAS)
	image.paste(img, (_x, (60+2*_y)))
	img = Image.open(ln[2])
	img = img.resize((60, 60), Image.ANTIALIAS)
	image.paste(img, ((60+2*_y), (60+2*_y)))
	return image
	
	
def cut4(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =4
	s={}
	s[0] = [_x ,_y]
	s[1] =[(_x*2 + 60) ,_y]
	s[2] = [_x ,(60+2*_y)]  
	s[3] = [(60+2*_y) ,(60+2*_y)]  
	
	for i in range(0,4):
		img = Image.open(ln[i])
		img = img.resize((60, 60), Image.ANTIALIAS)
		ll = s[i]
		image.paste(img, (int(ll[0]), int(ll[1])))
		print (i)
	return image
	
	
def cut5(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =3 
	s={}
	s[0] = [(132-40*2-_x)/2 , (132-40*2-_y)/2]  
	s[1] = [((132-40*2-_x)/2+40+_x),(132-40*2-_y)/2]  
	s[2] = [_x,((132-40*2-_x)/2+40+_y)]
	s[3] = [(_x*2+40),((132-40*2-_x)/2+40+_y)] 
	s[4] = [(_x*3+40*2),((132-40*2-_x)/2+40+_y)]  

	for i in range(0,5):
		img = Image.open(ln[i])
		img = img.resize((40, 40), Image.ANTIALIAS)
		ll = s[i]
		image.paste(img, (int(ll[0]), int(ll[1])))
		print (i)
	return image
	
	
def cut6(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =3 
	s={}
	_x = _y = 3;  
	s[0] = [_x ,((132-40*2-_x)/2)]
	s[1] = [(_x*2+40) ,((132-40*2-_x)/2)] 
	s[2] = [(_x*3+40*2) ,((132-40*2-_x)/2)]  
	s[3] = [_x ,((132-40*2-_x)/2+40+_y)]
	s[4] = [(_x*2+40) ,((132-40*2-_x)/2+40+_y)]  
	s[5] = [(_x*3+40*2) ,((132-40*2-_x)/2+40+_y)]

	for i in range(0,6):
		img = Image.open(ln[i])
		img = img.resize((40, 40), Image.ANTIALIAS)
		ll = s[i]
		image.paste(img, (int(ll[0]), int(ll[1])))
		print (i)
	return image
	
	
def cut7(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =3 
	s={}
 
	s[0] = [(132-40)/2 ,_y]
	s[1] = [_x ,(_y*2+40) ] 
	s[2] = [(_x*2+40) ,(_y*2+40)]
	s[3] = [(_x*3+40*2) ,(_y*2+40)]
	s[4] = [_x ,(_y*3+40*2)]
	s[5] = [(_x*2+40) ,(_y*3+40*2)]
	s[6] = [(_x*3+40*2) ,(_y*3+40*2)]

	for i in range(0,7):
		img = Image.open(ln[i])
		img = img.resize((40, 40), Image.ANTIALIAS)
		ll = s[i]
		image.paste(img, (int(ll[0]), int(ll[1])))
		print (i)
	return image
	
def cut8(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =3 
	s={}
	s[0] = [(132-80-_x)/2 ,_y]
	s[1] = [((132-80-_x)/2+_x+40) ,_y]
	s[2] = [_x ,(_y*2+40)]
	s[3] = [(_x*2+40) ,(_y*2+40)]
	s[4] = [(_x*3+40*2) ,(_y*2+40)]
	s[5] = [_x ,(_y*3+40*2)]
	s[6] = [(_x*2+40) ,(_y*3+40*2)]
	s[7] = [(_x*3+40*2) ,(_y*3+40*2)]

	for i in range(0,8):
		img = Image.open(ln[i])
		img = img.resize((40, 40), Image.ANTIALIAS)
		ll = s[i]
		image.paste(img, (int(ll[0]), int(ll[1])))
		print (i)
	return image
	
def cut9(ln):
	image = Image.new('RGB', (132, 132))
	draw = ImageDraw.Draw(image)
	# 填充每个像素:
	for x in range(132):
		for y in range(132):
			draw.point((x, y), (255, 255, 255))
	_x=_y =3 
	s={}
	s[0]=_x ,_y
	s[1] = [_x*2+40 ,_y]
	s[2] = [_x*3+40*2  ,_y]
	s[3] = [_x ,(_y*2+40)]
	s[4] = [(_x*2+40) ,(_y*2+40)]
	s[5] = [(_x*3+40*2) ,(_y*2+40)]
	s[6] = [_x ,(_y*3+40*2)]
	s[7] = [(_x*2+40) ,(_y*3+40*2)]
	s[8] = [(_x*3+40*2) ,(_y*3+40*2)]

	for i in range(0,9):
		img = Image.open(ln[i])
		img = img.resize((40, 40), Image.ANTIALIAS)
		ll = s[i]
		image.paste(img, (int(ll[0]), int(ll[1])))
	return image
if __name__ == "__main__":
	print ('hello')
	#img = Image.open(StringIO(data))
	#img = ima.convert("RGB")
	#image = cut4(ln)
	
	#image.show()
