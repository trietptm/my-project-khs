import urllib
import urllib2
import re
import string

PAGE_COUNT = 24


class CL4D2MapDown:
	def __init__(self):
		self.m_sHost = "http://difqjs.tistory.com"
		sUrl = 'http://difqjs.tistory.com/category/Left 4 Dead 2/L4D2 Maps?'
		self.m_sUrl = urllib.quote(sUrl,'/:?')
		self.m_p = re.compile(r'<li>\s*<span class="date">.*?\s*<a href=(.*?)>(.*?)</a>.*?\s</li>',re.DOTALL)
		self.m_Lmap = []
		
	def GetMapTitle(self,page):
		page = page + 1
		print page
		for n in range(1,page):
			print n
			ParamDick = {'page':n}
			sParamDick = urllib.urlencode(ParamDick)
			sUrlFullPath =  self.m_sUrl + sParamDick
			html = urllib2.urlopen(sUrlFullPath)
			sBody = html.read()
			#sBody = unicode(sBody,'utf-8').encode('cp949','ignore')
			li = self.m_p.findall(sBody)
			self.m_Lmap = self.m_Lmap + li	
			print "Get %d Page Tile" % n

		return self.m_Lmap

	def GetImagePath(self,a_sUrl):
		p = re.compile(r'<noscript>\s*(.*?)\s*</noscript>',re.DOTALL)
		html = urllib2.urlopen(a_sUrl)		
		s = html.read()
		li = p.findall(s)
		imglist = []
		for k in li:
			p = re.compile(r'<img src\s*=\s*(.*?)\s*alt')
			imglist = imglist + p.findall(k)

		return imglist

	def UrlTitleImg(self,index):
		L = self.m_Lmap[index]

		sUrl = self.m_sHost + L[0].replace('\"','')
		sTitle = L[1]
		sImg = self.GetImagePath(sUrl)

		f = open("map.html","a")		

		str = "sUrl : %s<br>" % sUrl
		str += "sTitle : %s<br>" % sTitle
		for k in sImg:
			str += "<img src=%s width=200 height=200>" % k

		str+= "<br>"

		f.write(str)

		f.close()

		print "Write %d img Count" % index
			
		
cMap = CL4D2MapDown()	
L = cMap.GetMapTitle(PAGE_COUNT)

nCount = len(L)
for k in range(nCount):
	print "Total Img : %d" % nCount
	cMap.UrlTitleImg(k)
	

