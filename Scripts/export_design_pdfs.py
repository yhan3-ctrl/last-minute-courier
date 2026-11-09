"""Render the course design documents; source Markdown remains editable."""
from pathlib import Path
import re
from xml.sax.saxutils import escape
from reportlab.pdfgen.canvas import Canvas
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'output/pdf'
OUT.mkdir(parents=True,exist_ok=True)
pdfmetrics.registerFont(TTFont('CourierBody',str(ROOT/'SourceAssets/Fonts/Nunito-Regular.ttf')))
pdfmetrics.registerFont(TTFont('CourierTitle',str(ROOT/'Content/Fonts/LilitaOne-Regular.ttf')))
INK=colors.HexColor('#263F43');TEAL=colors.HexColor('#39746F');ORANGE=colors.HexColor('#BE6343')
styles={
 'p':ParagraphStyle('Body',fontName='CourierBody',fontSize=10.2,leading=14.2,textColor=INK,spaceAfter=8),
 'h1':ParagraphStyle('Title',fontName='CourierTitle',fontSize=25,leading=30,textColor=INK,spaceAfter=15),
 'h2':ParagraphStyle('Heading',fontName='CourierTitle',fontSize=14.5,leading=19,textColor=TEAL,spaceBefore=14,spaceAfter=7,keepWithNext=True),
 'cell':ParagraphStyle('Cell',fontName='CourierBody',fontSize=8.3,leading=11,textColor=INK),
 'small':ParagraphStyle('Small',fontName='CourierBody',fontSize=8,leading=10,textColor=INK),
}
def fmt(s):
 s=s.replace('—','-').replace('–','-').replace('’',"'").replace('“','"').replace('”','"')
 s=escape(s)
 s=re.sub(r'\*\*(.*?)\*\*',r'<font name="CourierTitle">\1</font>',s)
 return s.replace('`','')
def para(s,style='p'):return Paragraph(fmt(s),styles[style])
def markdown(text):
 lines=text.splitlines();flow=[];i=0
 while i<len(lines):
  line=lines[i].strip()
  if not line:i+=1;continue
  if line.startswith('|'):
   rows=[]
   while i<len(lines) and lines[i].strip().startswith('|'):
    parts=[p.strip() for p in lines[i].strip().strip('|').split('|')]
    if not all(re.fullmatch(r'[-: ]+',p) for p in parts):rows.append(parts)
    i+=1
   n=len(rows[0]);width=500
   weights=([.19,.42,.39] if n==3 else [.15,.27,.30,.28] if n==4 else [1/n]*n)
   data=[[para(c,'cell') for c in row] for row in rows]
   t=Table(data,colWidths=[width*x for x in weights],repeatRows=1,hAlign='LEFT')
   t.setStyle(TableStyle([('VALIGN',(0,0),(-1,-1),'TOP'),('BACKGROUND',(0,0),(-1,0),colors.HexColor('#E6EFEB')),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,colors.HexColor('#F7F8F4')]),('LINEBELOW',(0,0),(-1,0),.7,TEAL),('LINEBELOW',(0,1),(-1,-1),.25,colors.HexColor('#D5DEDA')),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7)]))
   flow.extend([t,Spacer(1,10)]);continue
  if line.startswith('# '):flow.append(para(line[2:],'h1'))
  elif line.startswith('## '):flow.append(para(line[3:],'h2'))
  elif line.startswith('- '):flow.append(para('• '+line[2:]))
  else:flow.append(para(line))
  i+=1
 return flow

def footer(c,doc):
 c.setStrokeColor(TEAL);c.setLineWidth(.65);c.line(56,43,556,43)
 c.setFont('CourierBody',8);c.setFillColor(INK)
 c.drawString(56,29,'LAST MINUTE COURIER  |  Design v0.6  |  November 8, 2026')
 c.drawRightString(556,29,str(doc.page))
def export(name,paths):
 doc=SimpleDocTemplate(str(OUT/name),pagesize=(612,792),leftMargin=56,rightMargin=56,topMargin=48,bottomMargin=59,title=name.replace('.pdf',''),author='Yu Han')
 flow=[]
 for i,path in enumerate(paths):
  if i:flow.append(PageBreak())
  flow.extend(markdown((ROOT/path).read_text()))
 doc.build(flow,onFirstPage=footer,onLaterPages=footer)
 print(OUT/name)
export('LastMinuteCourier-HighConcept.pdf',['Documentation/HighConcept.md'])
export('LastMinuteCourier-GameScript-Graduate.pdf',['Documentation/GameScript.md','Documentation/GraduateAnalysis.md'])
