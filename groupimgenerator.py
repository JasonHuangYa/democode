
from cStringIO import StringIO
sys.setdefaultencoding("utf-8")
#from util.uploadqiniu import QiniuUpload
from PIL import Image
import json


# 传入1-n 张图片url
# 下载图片，生成仿微信群头像样式的新照片
def getgroupheadimg(imglist):
    dbobj = DBObj()
    data = dbobj.getquery(sql)
    picurlist = []
    ln=[]

    for url in imglist:
        #filter with weixin invalid url end with "/"
        if url.endswith('/'):
            continue
        else:
            picurlist.append(url)
            if len(picurlist) >=9:
                break
    for url in picurlist:
        f = urllib2.urlopen(url)
        data = f.read()
        ln.append(StringIO(data))
    sizel = len(ln)
    if sizel>9:
        size=9
    elif sizel<=0:
        return ''
    #  根据具体的图片数量，反射各个函数
    func = getattr(wxcrop, 'cut'+str(sizel))
    image = func(ln)
    image.show()
    value = StringIO()
    image.save(value, "JPEG")
    #filekey = hashlib.md5(value.getvalue()).hexdigest() + ".jpg"
    #furl = g_qiniuploadobj.uploadJPGFromBytes('chainlemochip', filekey, value.getvalue())
 

