PROJECT(Document)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

INCLUDE(UseLATEX.CMAKE)

SET(LATEX_OUTPUT_PATH "out")

ADD_LATEX_DOCUMENT(Document.tex
	BIBFILES bibliografia.bib
	INPUTS ch1.tex ch2.tex ch3.tex ch4.tex ch5.tex ch6.tex anex1.tex anex2.tex anex3.tex anex4.tex anex5.tex
	IMAGE_DIRS images
	IMAGES evolucion-gpu.png
	DEFAULT_PDF
	USE_METAPOST)