PROJECT(Document)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

INCLUDE(UseLATEX.CMAKE)

SET(LATEX_OUTPUT_PATH "out")


ADD_LATEX_DOCUMENT(Document.tex
	BIBFILES bibliografia.bib
	INPUTS 
		resumen.tex
		introduccion.tex
		funcione_resumen.tex
		tarjetas_graficas.tex
		mejoras.tex
		resultados.tex
		conclusiones.tex
		trabajos_futuros.tex
		presupuesto.tex
		entorno.tex
		manual.tex
		glosario.tex
		codigo.tex
		contenido_cd.tex
		aviso_legal.tex
		agradecimientos.tex
	IMAGE_DIRS images
	DEFAULT_PDF)
	
ADD_LATEX_DOCUMENT(presentation.tex
	BIBFILES bibliografia.bib
	IMAGE_DIRS images
	MANGLE_TARGET_NAMES
	DEFAULT_PDF)