"""Export the three course documents in a consistent Arial academic format."""
from pathlib import Path
import re
from xml.sax.saxutils import escape
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, CondPageBreak
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY, TA_LEFT
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'output/pdf'
FONT_ROOT = Path('/System/Library/Fonts/Supplemental')
for name, filename in [('Arial', 'Arial.ttf'), ('Arial-Bold', 'Arial Bold.ttf'),
                       ('Arial-Italic', 'Arial Italic.ttf'), ('Arial-BoldItalic', 'Arial Bold Italic.ttf')]:
    pdfmetrics.registerFont(TTFont(name, str(FONT_ROOT / filename)))
pdfmetrics.registerFontFamily('Arial', normal='Arial', bold='Arial-Bold',
                            italic='Arial-Italic', boldItalic='Arial-BoldItalic')
BLACK = colors.black
WIDTH = 468
styles = {
    'title': ParagraphStyle('Title', fontName='Arial', fontSize=18, leading=23,
                            alignment=TA_CENTER, spaceAfter=7),
    'subtitle': ParagraphStyle('Subtitle', fontName='Arial', fontSize=13, leading=18,
                               alignment=TA_CENTER, spaceAfter=16),
    'author': ParagraphStyle('Author', fontName='Arial', fontSize=11, leading=15,
                             alignment=TA_CENTER, spaceAfter=2),
    'h2': ParagraphStyle('Section', fontName='Arial-Bold', fontSize=12, leading=16,
                         spaceBefore=16, spaceAfter=7, keepWithNext=False),
    'h3': ParagraphStyle('Subsection', fontName='Arial-Bold', fontSize=11, leading=15,
                         spaceBefore=10, spaceAfter=5, keepWithNext=False),
    'p': ParagraphStyle('Body', fontName='Arial', fontSize=10.5, leading=14.4,
                        alignment=TA_JUSTIFY, firstLineIndent=16, spaceAfter=6, allowWidows=0, allowOrphans=0),
    'plain': ParagraphStyle('Plain', fontName='Arial', fontSize=10.5, leading=14.4,
                            alignment=TA_LEFT, spaceAfter=6, allowWidows=0, allowOrphans=0),
    'cell': ParagraphStyle('Cell', fontName='Arial', fontSize=8.8, leading=11.5,
                           alignment=TA_LEFT),
    'header': ParagraphStyle('Header', fontName='Arial-Bold', fontSize=8.8, leading=11.5),
}

def fmt(text):
    for a, b in [('—', '-'), ('–', '-'), ('’', "'"), ('“', '"'), ('”', '"')]:
        text = text.replace(a, b)
    text = escape(text.replace('`', ''))
    text = re.sub(r'\*\*(.*?)\*\*', r'<b>\1</b>', text)
    # Preserve selectable link labels and working PDF link annotations.
    text = re.sub(r'\[([^\]]+)\]\((https?://[^)]+)\)', r'<link href="\2">\1</link>', text)
    text = re.sub(r'(?<![\w"=])(https?://[^\s<]+)', r'<link href="\1">\1</link>', text)
    return text

def para(text, style='p'):
    return Paragraph(fmt(text), styles[style])

def markdown(text, section_start=0):
    lines = text.splitlines()
    flow = []
    i = 0
    section = section_start
    for_title = True
    while i < len(lines):
        line = lines[i].strip()
        if not line:
            i += 1
            continue
        if line.startswith('# '):
            # The exporter supplies a uniform title and author block.
            i += 1
            continue
        if for_title and not line.startswith('## '):
            i += 1
            continue
        for_title = False
        if line == '---PAGE---':
            flow.append(PageBreak()); i += 1; continue
        if line.startswith('|'):
            rows = []
            while i < len(lines) and lines[i].strip().startswith('|'):
                cells = [c.strip() for c in lines[i].strip().strip('|').split('|')]
                if not all(re.fullmatch(r'[-: ]+', c) for c in cells):
                    rows.append(cells)
                i += 1
            n = len(rows[0])
            weights = {2: [.25, .75], 3: [.19, .405, .405], 4: [.16, .28, .28, .28]}.get(n, [1/n]*n)
            data = [[para(c, 'header' if ri == 0 else 'cell') for c in row] for ri, row in enumerate(rows)]
            table = Table(data, colWidths=[WIDTH*w for w in weights], repeatRows=1, hAlign='LEFT')
            table.setStyle(TableStyle([
                ('VALIGN', (0, 0), (-1, -1), 'TOP'),
                ('LINEABOVE', (0, 0), (-1, 0), .65, BLACK),
                ('LINEBELOW', (0, 0), (-1, 0), .45, BLACK),
                ('LINEBELOW', (0, 1), (-1, -1), .2, colors.HexColor('#B8B8B8')),
                ('LEFTPADDING', (0, 0), (-1, -1), 5),
                ('RIGHTPADDING', (0, 0), (-1, -1), 5),
                ('TOPPADDING', (0, 0), (-1, -1), 5),
                ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
            ]))
            flow.extend([table, Spacer(1, 8)])
            continue
        if line.startswith('### '):
            flow.extend([CondPageBreak(95), para(line[4:], 'h3')])
        elif line.startswith('## '):
            section += 1
            title = re.sub(r'^\d+\.\s*', '', line[3:])
            flow.extend([CondPageBreak(120), para(f'{section}  {title}', 'h2')])
        elif line.startswith('- '):
            flow.append(para('• ' + line[2:], 'plain'))
        else:
            flow.append(para(line, 'plain' if line.startswith('**') else 'p'))
        i += 1
    return flow, section

def footer(canvas, doc):
    canvas.setFillColor(BLACK)
    canvas.setFont('Arial', 10)
    canvas.drawCentredString(306, 32, str(doc.page))

def export(name, subtitle, paths):
    OUT.mkdir(parents=True, exist_ok=True)
    doc = SimpleDocTemplate(str(OUT/name), pagesize=(612, 792),
                            leftMargin=72, rightMargin=72, topMargin=60, bottomMargin=55,
                            title='Last Minute Courier - '+subtitle, author='Yu Han',
                            subject='Project 3 game design', creator='Yu Han')
    flow = [Spacer(1, 22), para('Last Minute Courier', 'title'), para(subtitle, 'subtitle'),
            para('Yu Han', 'author'), para('University of Mississippi', 'author'),
            Spacer(1, 20)]
    section = 0
    for idx, path in enumerate(paths):
        if idx:
            flow.extend([PageBreak(), para('Graduate Mechanics and Balance Analysis', 'subtitle')])
        body, section = markdown((ROOT/path).read_text(), section)
        flow.extend(body)
    doc.build(flow, onFirstPage=footer, onLaterPages=footer)
    print(OUT/name)

if __name__ == '__main__':
    export('LastMinuteCourier-HighConcept.pdf', 'High Concept', ['Documentation/HighConcept.md'])
    export('LastMinuteCourier-GameScript-Graduate.pdf', 'Game Script and Graduate Analysis',
           ['Documentation/GameScript.md', 'Documentation/GraduateAnalysis.md'])
    export('LastMinuteCourier-Report.pdf', 'Project 3: Design and Iteration Report', ['Documentation/Report.md'])
