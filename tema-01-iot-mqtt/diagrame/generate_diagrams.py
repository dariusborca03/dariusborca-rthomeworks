from pathlib import Path
from xml.sax.saxutils import escape
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw, ImageFont
import math
ROOT=Path(__file__).parent
W,H=1440,850
BLUE='#2563eb'; INK='#172033'; GRAY='#64748b'; GREEN='#15803d'; RED='#b91c1c'
fontpath='C:/Windows/Fonts/arial.ttf'
class Diagram:
 def __init__(self,title,subtitle):
  self.title=title; self.items=[]
  self.text(40,28,title,28,INK,'left'); self.text(40,72,subtitle,17,GRAY,'left')
 def rect(self,x,y,w,h,label='',fill='#eff6ff',stroke=BLUE,size=18):
  self.items.append(('rect',x,y,w,h,fill,stroke))
  if label:self.text(x+w/2,y+h/2-10*len(label.split('\n')),label,size)
 def text(self,x,y,t,size=18,color=INK,anchor='middle'):
  self.items.append(('text',x,y,t,size,color,anchor))
 def line(self,x1,y1,x2,y2,color=GRAY,arrow=False,dash=False):
  self.items.append(('line',x1,y1,x2,y2,color,arrow,dash))
 def message(self,x1,x2,y,label,color=BLUE,dash=False):
  self.line(x1,y,x2,y,color,True,dash); self.text((x1+x2)/2,y-44,label,16,color)
 def save(self,name):
  img=Image.new('RGB',(W,H),'white'); d=ImageDraw.Draw(img)
  svg=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}"><rect width="100%" height="100%" fill="white"/>']
  mx=ET.Element('mxfile',{'host':'app.diagrams.net','agent':'Darius Borca','version':'24.7.17'}); dg=ET.SubElement(mx,'diagram',{'name':self.title,'id':name}); model=ET.SubElement(dg,'mxGraphModel',{'dx':'1440','dy':'850','grid':'1','gridSize':'10','page':'1','pageWidth':str(W),'pageHeight':str(H)}); root=ET.SubElement(model,'root');ET.SubElement(root,'mxCell',{'id':'0'});ET.SubElement(root,'mxCell',{'id':'1','parent':'0'})
  for idx,item in enumerate(self.items,2):
   typ,*a=item
   if typ=='rect':
    x,y,w,h,fill,stroke=a;svg.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="8" fill="{fill}" stroke="{stroke}" stroke-width="2"/>');d.rounded_rectangle((x,y,x+w,y+h),8,fill=fill,outline=stroke,width=2)
    cell=ET.SubElement(root,'mxCell',{'id':str(idx),'value':'','style':f'rounded=1;whiteSpace=wrap;html=0;fillColor={fill};strokeColor={stroke};','vertex':'1','parent':'1'});ET.SubElement(cell,'mxGeometry',{'x':str(x),'y':str(y),'width':str(w),'height':str(h),'as':'geometry'})
   elif typ=='text':
    x,y,t,size,color,anchor=a; font=ImageFont.truetype(fontpath,size); lines=t.split('\n');lh=size+5
    for k,line in enumerate(lines):
     yy=y+k*lh;svg.append(f'<text x="{x}" y="{yy+size}" text-anchor="'+('start' if anchor=='left' else 'middle')+f'" font-family="Arial,sans-serif" font-size="{size}" fill="{color}">{escape(line)}</text>');width=d.textlength(line,font=font);d.text((x if anchor=='left' else x-width/2,yy),line,fill=color,font=font)
    cell=ET.SubElement(root,'mxCell',{'id':str(idx),'value':t,'style':f'text;whiteSpace=wrap;html=0;align='+('left' if anchor=='left' else 'center')+f';verticalAlign=top;fontSize={size};fontColor={color};','vertex':'1','parent':'1'});ET.SubElement(cell,'mxGeometry',{'x':str(x if anchor=='left' else x-500),'y':str(y),'width':'1000','height':str(len(lines)*lh+10),'as':'geometry'})
   elif typ=='line':
    x1,y1,x2,y2,color,arrow,dash=a
    svg.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" stroke-width="2"'+(' stroke-dasharray="6 5"' if dash else '')+'/>')
    if dash:
     length=math.hypot(x2-x1,y2-y1)
     if length:
      for off in range(0,int(length),11):
       e=min(off+6,length);d.line((x1+(x2-x1)*off/length,y1+(y2-y1)*off/length,x1+(x2-x1)*e/length,y1+(y2-y1)*e/length),fill=color,width=2)
    else:d.line((x1,y1,x2,y2),fill=color,width=2)
    if arrow:
     ang=math.atan2(y2-y1,x2-x1);pts=[(x2,y2),(x2-12*math.cos(ang-.45),y2-12*math.sin(ang-.45)),(x2-12*math.cos(ang+.45),y2-12*math.sin(ang+.45))];svg.append('<polygon points="'+' '.join(f'{x},{y}' for x,y in pts)+f'" fill="{color}"/>');d.polygon(pts,fill=color)
    cell=ET.SubElement(root,'mxCell',{'id':str(idx),'value':'','style':f'endArrow='+('block' if arrow else 'none')+f';strokeColor={color};dashed={int(dash)};','edge':'1','parent':'1'});geo=ET.SubElement(cell,'mxGeometry',{'relative':'1','as':'geometry'});ET.SubElement(geo,'mxPoint',{'x':str(x1),'y':str(y1),'as':'sourcePoint'});ET.SubElement(geo,'mxPoint',{'x':str(x2),'y':str(y2),'as':'targetPoint'})
  svg.append('</svg>');(ROOT/(name+'.svg')).write_text('\n'.join(svg),encoding='utf-8');img.save(ROOT/(name+'.png'));ET.indent(mx);ET.ElementTree(mx).write(ROOT/(name+'.drawio'),encoding='utf-8',xml_declaration=True)

def sequence(title,subtitle,participants):
 d=Diagram(title,subtitle);xs=[100+248*i for i in range(len(participants))]
 for x,p in zip(xs,participants):d.rect(x-95,115,190,75,p);d.line(x,190,x,810,GRAY,False,True)
 return d,xs

d=Diagram('Tema 01 · Diagrama de componente','Darius Borca · Namespace MQTT: ssatr/borca-darius/devices/<id>/ · QoS 1')
d.rect(30,130,440,580,'',fill='#f8fafc',stroke=GRAY);d.text(250,150,'«component» device-simulator',23)
d.rect(65,215,370,90,'PeriodicTelemetryTask\nsenzor → telemetry');d.rect(65,390,370,90,'SporadicAlarmTask\nMIN_INTERARRIVAL = 5000 ms');d.rect(65,570,370,90,'AperiodicCommandTask\nSET_INTERVAL / SET_THRESHOLD\nRESET / STATUS',size=16)
d.line(180,305,180,390,BLUE,True);d.text(290,330,'BlockingQueue<AlarmEvent>',15)
d.rect(590,320,230,130,'«component»\nBroker MQTT\ncontrol.aut.utcluj.ro:11188',size=17)
d.rect(940,130,470,580,'',fill='#f8fafc',stroke=GRAY);d.text(1175,150,'«component» business-service',23)
d.rect(990,215,370,100,'MessageProcessor × 3\ntelemetry / alarm / status')
d.rect(990,395,165,90,'CsvStorage\ndata/*.csv',size=17);d.rect(1190,395,170,90,'Dashboard\nshow',size=17)
d.rect(990,570,370,90,'CommandConsole\noperator → commands',size=18)
d.line(1175,315,1175,355,BLUE);d.line(1072,355,1275,355,BLUE);d.line(1072,355,1072,395,BLUE,True);d.line(1275,355,1275,395,BLUE,True)
d.line(435,260,540,260,BLUE);d.line(540,260,540,350,BLUE);d.line(540,350,590,350,BLUE,True);d.text(515,215,'telemetry',15)
d.line(435,435,590,405,BLUE,True);d.text(515,445,'alarm',16)
d.line(435,610,565,610,BLUE);d.line(565,610,565,430,BLUE);d.line(565,430,590,430,BLUE,True);d.text(495,630,'status',16)
d.line(820,355,900,355,BLUE);d.line(900,355,900,265,BLUE);d.line(900,265,990,265,BLUE,True);d.text(885,205,'abonări +',16)
d.line(990,620,855,620,GREEN);d.line(855,620,855,425,GREEN);d.line(855,425,820,425,GREEN,True);d.text(903,640,'commands',16)
d.line(705,450,705,730,GREEN);d.line(705,730,250,730,GREEN);d.line(250,730,250,660,GREEN,True);d.text(510,750,'MQTT → coada commands → task aperiodic',17,GREEN)
d.save('componente')

d,x=sequence('Tema 01 · Secvență: telemetrie și alarmă','Darius Borca · Exemplu: 30.00 °C > pragul 28.00 °C · topic-uri sub ssatr/borca-darius/devices/device-1/', ['Periodic','Senzor','Coada alarme','Sporadic','Broker MQTT','Business service'])
d.message(x[0],x[1],245,'read()');d.message(x[1],x[0],315,'30.00 °C',GRAY,True)
d.message(x[0],x[4],390,'publish telemetry: device-1;timestamp;30.00');d.message(x[4],x[5],450,'telemetry\nCSV + Dashboard')
d.message(x[0],x[2],520,'offer(30.00, 28.00, arrivalNanos)');d.message(x[2],x[3],590,'take() → eveniment')
d.text(x[3],620,'verifică intervalul\nacceptă prima alarmă',15,GREEN)
d.message(x[3],x[4],715,'publish alarm\n30.00;28.00');d.message(x[4],x[5],775,'alarm → CSV\ncontor + afișare')
d.save('secventa-alarma')

d,x=sequence('Tema 01 · Secvență: SET_INTERVAL și confirmare','Darius Borca · Comanda: device-1 SET_INTERVAL 1000 · Callback-ul MQTT pune textul în coadă, fără execuție', ['Operator','Business service','Broker MQTT','Coada comenzi','Aperiodic','Periodic'])
d.message(x[0],x[1],245,'device-1\nSET_INTERVAL 1000');d.message(x[1],x[2],330,'publish commands\nSET_INTERVAL 1000')
d.message(x[2],x[3],420,'callback → offer(text)');d.message(x[3],x[4],505,'take()');d.message(x[4],x[5],590,'setPeriodMs(1000)\nvalidare + scriere volatile')
d.message(x[4],x[2],680,'publish status: device-1;timestamp;OK SET_INTERVAL 1000');d.message(x[2],x[1],770,'status → MessageProcessor → CSV + afișare')
d.save('secventa-comanda')

d=Diagram('Tema 01 · Diagrama de timp (0–10 s)','Darius Borca · Scenariu ilustrativ, nu măsurători · T periodic = 2 s · Interval minim sporadic = 5 s')
x0=270;scale=108
for t in range(11):
 x=x0+t*scale;d.line(x,155,x,675,'#e2e8f0',False,True);d.text(x,125,str(t)+' s',17)
for y,label in [(275,'Periodic\nT = 2000 ms'),(440,'Sporadic\nalarme'),(610,'Aperiodic\ncomenzi')]:d.text(40,y-35,label,20,INK,'left');d.line(x0,y,x0+1080,y,GRAY,True)
for t in range(0,11,2):
 x=x0+t*scale;d.line(x,285,x,235,BLUE,True);d.rect(x+2,245,24,30,'',fill='#93c5fd');d.text(x,200,'r'+str(t//2),17,BLUE)
for t,accept in [(2.1,True),(4.1,False),(6.1,False),(8.1,True)]:
 x=x0+t*scale;c=GREEN if accept else RED;d.line(x,460,x,405,c,True)
 if accept:d.rect(x+2,410,26,30,'',fill='#bbf7d0',stroke=c)
 else:d.text(x,405,'×',27,c)
 d.text(x,480,('acceptată' if accept else 'respinsă')+'\n'+str(t)+' s',16,c)
x=x0+7*scale;d.line(x,630,x,570,BLUE,True);d.rect(x+2,580,35,30,'',fill='#bfdbfe');d.text(x,645,'STATUS la 7 s',17,BLUE)
d.text(40,715,'Legendă: ↑ eliberare / sosire; dreptunghi = job; × = eveniment respins înainte de publicarea alarmei.',18,INK,'left')
d.text(40,753,'Alarme la 2.1 s și 8.1 s: Δ = 6 s ≥ 5 s. Cele de la 4.1 s și 6.1 s sunt prea apropiate de ultima acceptată.',18,GRAY,'left')
d.text(40,791,'Lățimile joburilor sunt simbolice. Thread-urile Java și MQTT nu oferă garanții hard real-time.',17,GRAY,'left')
d.save('timp')
(ROOT/'README.md').write_text('''# Diagrame — Darius Borca

Fiecare diagramă are o sursă draw.io editabilă (`.drawio`, XML necomprimat), un export vectorial (`.svg`) și un export PNG (`.png`). Deschideți fișierele draw.io în diagrams.net.

- [Componente](componente.png): simulator, taskuri, cozi, broker și serviciu business.
- [Secvența alarmei](secventa-alarma.png): telemetrie peste prag și prima alarmă acceptată.
- [Secvența comenzii](secventa-comanda.png): `SET_INTERVAL 1000`, execuție aperiodică și confirmare.
- [Timp](timp.png): scenariu ilustrativ de 10 secunde, cu eliberări periodice, alarme acceptate/respinse și comanda `STATUS`.

Diagrama de timp este un model explicativ, nu un rezultat măsurat. Duratele joburilor nu sunt la scară; perioadele și intervalul minim dintre evenimente sunt la scară. În secvența comenzii, coloana `Business service` include `CommandConsole` pentru trimitere și `MessageProcessor` pentru confirmare.

`generate_diagrams.py` regenerează sursele și exporturile din același model geometric. Necesită Python 3, Pillow și fontul Arial din Windows. Rulare: `python generate_diagrams.py`.
''',encoding='utf-8')
