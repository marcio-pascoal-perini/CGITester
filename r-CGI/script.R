#!/usr/bin/env Rscript

library(knitr)

args <- Sys.getenv('QUERY_STRING')
pairs <- strsplit(args, '&')[[1]]
adjusted_list <- strsplit(pairs, '=')
values <- sapply(adjusted_list, function(x) URLdecode(x[2]))
names <- sapply(adjusted_list, function(x) x[1])
names(values) <- names
data <- as.list(values)

args <- commandArgs(trailingOnly = FALSE)
file_arg <- grep('--file=', args, value = TRUE)
caminho_completo <- sub("--file=", "", file_arg)
diretorio_script <- dirname(caminho_completo)
diretorio_script <- normalizePath(diretorio_script)
setwd(diretorio_script)

if (data$ABG == 1) {
    load('dados-estados-populacao.Rda')
    result <- subset(dadosEstadosPopulacao, D2C == data$D2C)
} else {
    load('dados-regioes-populacao.Rda')
    result <- subset(dadosRegioesPopulacao, D2C == data$D2C)
}

cat('Access-Control-Allow-Origin: *\n')
cat('Access-Control-Allow-Headers: *\n')
cat('Access-Control-Allow-Methods: *\n')
cat('Content-Type: text/html; charset=utf-8\n')
cat('\n')
cat('<html>\n')
cat('<head>\n')
cat('<style>\n')
cat('table {width: 100%; margin-bottom: 1rem; color: #212529; border-color: #dee2e6; border-collapse: collapse;}\n')
cat('table th, table td {padding: 0.5rem 0.5rem; border-bottom: 1px solid #dee2e6;}\n')
cat('table th {border-bottom: 2px solid #dee2e6;}\n')
cat('</style>\n')
cat('</head>\n')
cat('<body>\n')
kable(result, format = 'html', escape = FALSE)
cat('</body>\n')
cat('</html>\n')
