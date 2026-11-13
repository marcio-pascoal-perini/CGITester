
/*
 * Windows
 *
 * gcc horoscopo.c -o horoscopo.exe -mwindows
 *
 * Linux
 *
 * gcc horoscopo.c -o horoscopo.exe
  *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_TEXT 1024

typedef struct
{
    const char *id;
    const char *nome;
    const char *simbolo;
    const char *periodo;
} SignoInfo;

static const SignoInfo LISTA_SIGNOS[] = {
    {"aries", "Áries", "&#9800;", "21/03 a 19/04"},
    {"touro", "Touro", "&#9801;", "20/04 a 20/05"},
    {"gemeos", "Gêmeos", "&#9802;", "21/05 a 20/06"},
    {"cancer", "Câncer", "&#9803;", "21/06 a 22/07"},
    {"leao", "Leão", "&#9804;", "23/07 a 22/08"},
    {"virgem", "Virgem", "&#9805;", "23/08 a 22/09"},
    {"libra", "Libra", "&#9806;", "23/09 a 22/10"},
    {"escorpiao", "Escorpião", "&#9807;", "23/10 a 21/11"},
    {"sagitario", "Sagitário", "&#9808;", "22/11 a 21/12"},
    {"capricornio", "Capricórnio", "&#9809;", "22/12 a 19/01"},
    {"aquario", "Aquário", "&#9810;", "20/01 a 18/02"},
    {"peixes", "Peixes", "&#9811;", "19/02 a 20/03"}};

#define TOTAL_SIGNOS (sizeof(LISTA_SIGNOS) / sizeof(LISTA_SIGNOS[0]))

void normalizar_string(const char *entrada, char *saida, size_t max_len)
{
    size_t i = 0, j = 0;
    while (entrada[i] != 0 && j < max_len - 1)
    {
        unsigned char c = (unsigned char)entrada[i];
        if (c == 0xC3 && entrada[i + 1] != 0)
        {
            unsigned char next = (unsigned char)entrada[i + 1];
            switch (next)
            {
            case 0xA1:
            case 0xA0:
            case 0xA2:
            case 0xA3:
            case 0x81:
            case 0x80:
            case 0x82:
            case 0x83:
                saida[j++] = 'a';
                break;
            case 0xA9:
            case 0xAA:
            case 0x89:
            case 0x8A:
                saida[j++] = 'e';
                break;
            case 0xAD:
            case 0x8D:
                saida[j++] = 'i';
                break;
            case 0xB3:
            case 0xB4:
            case 0xB5:
            case 0x93:
            case 0x94:
            case 0x95:
                saida[j++] = 'o';
                break;
            case 0xBA:
            case 0x9A:
                saida[j++] = 'u';
                break;
            case 0xA7:
            case 0x87:
                saida[j++] = 'c';
                break;
            default:
                break;
            }
            i += 2;
        }
        else if (c >= 0x80)
        {
            switch (c)
            {
            case 0xA0:
            case 0x85:
            case 0x83:
            case 0xC6:
            case 0xB5:
            case 0xB7:
            case 0xB6:
            case 0xE1:
            case 0xE0:
            case 0xE2:
            case 0xE3:
            case 0xC1:
            case 0xC0:
            case 0xC2:
            case 0xC3:
                saida[j++] = 'a';
                break;
            case 0x82:
            case 0x88:
            case 0x90:
            case 0xD2:
            case 0xE9:
            case 0xEA:
            case 0xC9:
            case 0xCA:
                saida[j++] = 'e';
                break;
            case 0xA1:
            case 0xD6:
            case 0xED:
            case 0xCD:
                saida[j++] = 'i';
                break;
            case 0xA2:
            case 0x93:
            case 0xE4:
            case 0xE5:
            case 0xF3:
            case 0xF4:
            case 0xF5:
            case 0xD3:
            case 0xD4:
            case 0xD5:
                saida[j++] = 'o';
                break;
            case 0xA3:
            case 0xFA:
            case 0xDA:
                saida[j++] = 'u';
                break;
            case 0x87:
            case 0x80:
            case 0xE7:
            case 0xC7:
                saida[j++] = 'c';
                break;
            default:
                break;
            }
            i++;
        }
        else
        {
            if (!isspace(c))
            {
                saida[j++] = (char)tolower(c);
            }
            i++;
        }
    }
    saida[j] = 0;
}

void url_decode(const char *src, char *dest, size_t max_len)
{
    size_t i = 0, j = 0;
    while (src[i] != 0 && j < max_len - 1)
    {
        if (src[i] == '+')
        {
            dest[j++] = ' ';
            i++;
        }
        else if (src[i] == '%' && isxdigit((unsigned char)src[i + 1]) && isxdigit((unsigned char)src[i + 2]))
        {
            char hex[3] = {src[i + 1], src[i + 2], 0};
            dest[j++] = (char)strtol(hex, NULL, 16);
            i += 3;
        }
        else
        {
            dest[j++] = src[i++];
        }
    }
    dest[j] = 0;
}

int obter_parametro_cgi(const char *query, const char *chave, char *saida, size_t max_len)
{
    if (!query || !chave)
        return 0;
    size_t tam_chave = strlen(chave);
    const char *p = query;

    while (*p != 0)
    {
        if (strncmp(p, chave, tam_chave) == 0 && p[tam_chave] == '=')
        {
            p += tam_chave + 1;
            char valor_codificado[256] = {0};
            size_t k = 0;
            while (p[k] != 0 && p[k] != '&' && k < sizeof(valor_codificado) - 1)
            {
                valor_codificado[k] = p[k];
                k++;
            }
            valor_codificado[k] = 0;
            url_decode(valor_codificado, saida, max_len);
            return 1;
        }
        p = strchr(p, '&');
        if (!p)
            break;
        p++;
    }
    return 0;
}

int extrair_tag_xml_intervalo(const char *inicio_bloco, const char *fim_bloco, const char *tag, char *destino, size_t max_len)
{
    char tag_abertura[64], tag_fechamento[64];
    snprintf(tag_abertura, sizeof(tag_abertura), "<%s>", tag);
    snprintf(tag_fechamento, sizeof(tag_fechamento), "</%s>", tag);

    const char *inicio = strstr(inicio_bloco, tag_abertura);
    if (!inicio || inicio >= fim_bloco)
        return 0;
    inicio += strlen(tag_abertura);

    const char *fim = strstr(inicio, tag_fechamento);
    if (!fim || fim > fim_bloco)
        return 0;

    size_t tamanho = (size_t)(fim - inicio);
    if (tamanho >= max_len)
    {
        tamanho = max_len - 1;
    }

    memcpy(destino, inicio, tamanho);
    destino[tamanho] = 0;
    return 1;
}

void imprimir_html_escapado(const char *texto)
{
    for (size_t i = 0; texto[i] != 0; i++)
    {
        unsigned char c = (unsigned char)texto[i];
        if (c == '&')
        {
            fputs("&amp;", stdout);
        }
        else if (c == '<')
        {
            fputs("&lt;", stdout);
        }
        else if (c == '>')
        {
            fputs("&gt;", stdout);
        }
        else if (c == 34)
        {
            fputs("&quot;", stdout);
        }
        else if (c == 39)
        {
            fputs("&#39;", stdout);
        }
        else
        {
            fputc(c, stdout);
        }
    }
}

const char *obter_hex_cor(const char *cor_nome)
{
    char cor_norm[64] = {0};
    normalizar_string(cor_nome, cor_norm, sizeof(cor_norm));

    if (strstr(cor_norm, "vermelho"))
        return "#ef4444";
    if (strstr(cor_norm, "verdeesmeralda"))
        return "#10b981";
    if (strstr(cor_norm, "amarelosolar"))
        return "#facc15";
    if (strstr(cor_norm, "brancoperola"))
        return "#f8fafc";
    if (strstr(cor_norm, "dourado"))
        return "#eab308";
    if (strstr(cor_norm, "azulmarinho"))
        return "#1e3a8a";
    if (strstr(cor_norm, "rosaquartzo"))
        return "#f472b6";
    if (strstr(cor_norm, "vinho"))
        return "#881337";
    if (strstr(cor_norm, "roxolavanda"))
        return "#a855f7";
    if (strstr(cor_norm, "marromterra"))
        return "#78350f";
    if (strstr(cor_norm, "azulturquesa"))
        return "#06b6d4";
    if (strstr(cor_norm, "lilas"))
        return "#c084fc";
    if (strstr(cor_norm, "laranjaambar"))
        return "#f97316";
    if (strstr(cor_norm, "prata"))
        return "#cbd5e1";
    if (strstr(cor_norm, "verdeoliva"))
        return "#65a30d";
    if (strstr(cor_norm, "azulceleste"))
        return "#38bdf8";
    if (strstr(cor_norm, "coral"))
        return "#fb7185";
    if (strstr(cor_norm, "begeareia"))
        return "#fde68a";
    if (strstr(cor_norm, "bordo"))
        return "#7f1d1d";
    if (strstr(cor_norm, "verdeagua"))
        return "#2dd4bf";
    if (strstr(cor_norm, "magenta"))
        return "#db2777";
    if (strstr(cor_norm, "salmao"))
        return "#fda4af";
    if (strstr(cor_norm, "indigo"))
        return "#6366f1";
    if (strstr(cor_norm, "cobre"))
        return "#d97706";
    if (strstr(cor_norm, "amarelomostarda"))
        return "#ca8a04";
    if (strstr(cor_norm, "cinzagrafite"))
        return "#64748b";
    if (strstr(cor_norm, "violeta"))
        return "#8b5cf6";
    if (strstr(cor_norm, "azulroyal"))
        return "#2563eb";
    if (strstr(cor_norm, "verdementa"))
        return "#34d399";
    if (strstr(cor_norm, "terracota"))
        return "#ea580c";
    if (strstr(cor_norm, "caramelo"))
        return "#b45309";
    return "#fbbf24";
}

FILE *abrir_arquivo_xml(const char *argv0)
{
    FILE *f = fopen("horoscopo.xml", "rb");
    if (f)
        return f;

    f = fopen("../horoscopo.xml", "rb");
    if (f)
        return f;

    if (argv0 && argv0[0] != 0)
    {
        char caminho[512] = {0};
        strncpy(caminho, argv0, sizeof(caminho) - 1);
        char *barra1 = strrchr(caminho, 92); /* '' */
        char *barra2 = strrchr(caminho, '/');
        char *ultima_barra = (barra1 > barra2) ? barra1 : barra2;
        if (ultima_barra)
        {
            *(ultima_barra + 1) = 0;
            strncat(caminho, "horoscopo.xml", sizeof(caminho) - strlen(caminho) - 1);
            f = fopen(caminho, "rb");
            if (f)
                return f;
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    char entrada_usuario[128] = {0};
    char signo_busca[128] = {0};
    char query_buffer[512] = {0};

    const char *metodo = getenv("REQUEST_METHOD");
    const char *query_env = getenv("QUERY_STRING");

    if (metodo && strcmp(metodo, "POST") == 0)
    {
        const char *content_len_str = getenv("CONTENT_LENGTH");
        int content_len = content_len_str ? atoi(content_len_str) : 0;
        if (content_len > 0)
        {
            if (content_len >= (int)sizeof(query_buffer))
            {
                content_len = (int)sizeof(query_buffer) - 1;
            }
            size_t lidos_post = fread(query_buffer, 1, (size_t)content_len, stdin);
            query_buffer[lidos_post] = 0;
            obter_parametro_cgi(query_buffer, "signo", entrada_usuario, sizeof(entrada_usuario));
        }
    }

    if (entrada_usuario[0] == 0 && query_env && query_env[0] != 0)
    {
        if (!obter_parametro_cgi(query_env, "signo", entrada_usuario, sizeof(entrada_usuario)))
        {
            if (strchr(query_env, '=') == NULL)
            {
                url_decode(query_env, entrada_usuario, sizeof(entrada_usuario));
            }
        }
    }

    if (entrada_usuario[0] == 0 && argc > 1)
    {
        strncpy(entrada_usuario, argv[1], sizeof(entrada_usuario) - 1);
    }

    normalizar_string(entrada_usuario, signo_busca, sizeof(signo_busca));

    time_t agora = time(NULL);
    struct tm *data_hoje = localtime(&agora);
    int dia_atual = data_hoje->tm_mday;

    puts("Content-Type: text/html; charset=UTF-8");
    puts("");

    puts("<!DOCTYPE html>");
    puts("<html lang='pt-BR'>");
    puts("<head>");
    puts("<meta charset='UTF-8'>");
    puts("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    puts("<title>Horóscopo do Dia</title>");
    puts("<style>");
    puts("  * { box-sizing: border-box; margin: 0; padding: 0; }");
    puts("  html, body { width: 100%; height: 100%; font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; background: transparent; color: #f8fafc; overflow-x: hidden; }");
    puts("  .widget { width: 100%; min-height: 100%; background: linear-gradient(145deg, #1e1b4b 0%, #311042 55%, #0f172a 100%); border-radius: 12px; padding: 12px 14px; display: flex; flex-direction: column; justify-content: space-between; border: 1px solid rgba(255,255,255,0.14); box-shadow: inset 0 1px 0 rgba(255,255,255,0.1); }");
    puts("  .header { display: flex; align-items: center; justify-content: space-between; border-bottom: 1px solid rgba(255,255,255,0.12); padding-bottom: 8px; margin-bottom: 8px; }");
    puts("  .title { font-size: 14px; font-weight: 700; letter-spacing: 0.4px; color: #fde047; display: flex; align-items: center; gap: 5px; text-transform: uppercase; }");
    puts("  .date { font-size: 11px; color: #cbd5e1; background: rgba(255,255,255,0.08); padding: 2px 7px; border-radius: 999px; font-weight: 500; }");
    puts("  .form-bar { display: flex; gap: 6px; margin-bottom: 9px; }");
    puts("  select { flex: 1; background: rgba(15, 23, 42, 0.75); color: #f8fafc; border: 1px solid rgba(255,255,255,0.22); border-radius: 7px; padding: 5px 8px; font-size: 12.5px; outline: none; cursor: pointer; transition: border-color 0.2s; }");
    puts("  select:focus, select:hover { border-color: #fde047; }");
    puts("  select option { background: #1e1b4b; color: #f8fafc; }");
    puts("  button { background: #eab308; color: #0f172a; border: none; border-radius: 7px; padding: 5px 10px; font-size: 12px; font-weight: 700; cursor: pointer; transition: background 0.2s; }");
    puts("  button:hover { background: #facc15; }");
    puts("  .content { flex: 1; display: flex; flex-direction: column; justify-content: center; }");
    puts("  .sign-header { display: flex; align-items: center; gap: 8px; margin-bottom: 6px; }");
    puts("  .sign-icon { width: 30px; height: 30px; border-radius: 8px; background: rgba(253, 224, 71, 0.14); border: 1px solid rgba(253, 224, 71, 0.35); display: flex; align-items: center; justify-content: center; font-size: 17px; color: #fde047; flex-shrink: 0; }");
    puts("  .sign-name { font-size: 15px; font-weight: 700; color: #ffffff; line-height: 1.1; }");
    puts("  .sign-period { font-size: 10.5px; color: #94a3b8; }");
    puts("  .previsao { font-size: 12.5px; line-height: 1.42; color: #e2e8f0; background: rgba(255,255,255,0.05); padding: 8px 10px; border-radius: 8px; border-left: 3px solid #fde047; margin-bottom: 8px; }");
    puts("  .meta { display: grid; grid-template-columns: 1fr 1fr; gap: 6px; }");
    puts("  .badge { background: rgba(15, 23, 42, 0.55); border: 1px solid rgba(255,255,255,0.1); border-radius: 7px; padding: 5px 8px; display: flex; flex-direction: column; gap: 1px; }");
    puts("  .badge-label { font-size: 10px; text-transform: uppercase; letter-spacing: 0.4px; color: #94a3b8; }");
    puts("  .badge-val { font-size: 12.5px; font-weight: 700; color: #f8fafc; display: flex; align-items: center; gap: 5px; }");
    puts("  .color-dot { width: 10px; height: 10px; border-radius: 50%; display: inline-block; border: 1px solid rgba(255,255,255,0.5); flex-shrink: 0; }");
    puts("  .empty-state { text-align: center; padding: 14px 8px; color: #cbd5e1; font-size: 12.5px; line-height: 1.45; }");
    puts("  .empty-icons { font-size: 18px; letter-spacing: 4px; color: #fde047; margin-bottom: 6px; opacity: 0.9; }");
    puts("  .error-box { background: rgba(239, 68, 68, 0.18); border: 1px solid rgba(239, 68, 68, 0.4); color: #fecaca; padding: 8px 10px; border-radius: 8px; font-size: 12px; line-height: 1.4; }");
    puts("</style>");
    puts("</head>");
    puts("<body>");
    puts("<div class='widget'>");

    puts("  <div>");
    puts("    <div class='header'>");
    puts("      <div class='title'>&#10024; Horóscopo do Dia</div>");
    printf("      <div class='date'>%02d/%02d/%04d</div>",
           data_hoje->tm_mday,
           data_hoje->tm_mon + 1,
           data_hoje->tm_year + 1900);
    fputc(10, stdout);
    puts("    </div>");

    puts("    <form class='form-bar' method='get' action=''>");
    puts("      <select name='signo' aria-label='Selecione o seu signo' onchange='this.form.submit()'>");
    puts("        <option value=''>Escolha seu signo...</option>");
    for (size_t i = 0; i < TOTAL_SIGNOS; i++)
    {
        int selecionado = (strcmp(signo_busca, LISTA_SIGNOS[i].id) == 0);
        printf("        <option value='%s'%s>%s (%s)</option>",
               LISTA_SIGNOS[i].id,
               selecionado ? " selected" : "",
               LISTA_SIGNOS[i].nome,
               LISTA_SIGNOS[i].periodo);
        fputc(10, stdout);
    }
    puts("      </select>");
    puts("      <button type='submit'>Ver</button>");
    puts("    </form>");
    puts("  </div>");

    puts("  <div class='content'>");

    if (strlen(signo_busca) == 0)
    {
        puts("    <div class='empty-state'>");
        puts("      <div class='empty-icons'>&#9800;&#9804;&#9808;&#9811;</div>");
        puts("      Selecione o seu signo acima para conferir a previsão, o número da sorte e a cor de hoje.");
        puts("    </div>");
        puts("  </div>");
        puts("</div>");
        puts("</body>");
        puts("</html>");
        return 0;
    }

    FILE *arquivo = abrir_arquivo_xml(argv[0]);
    if (!arquivo)
    {
        puts("    <div class='error-box'>Erro: Não foi possível abrir o arquivo <strong>horoscopo.xml</strong>.</div>");
        puts("  </div></div></body></html>");
        return 1;
    }

    fseek(arquivo, 0, SEEK_END);
    long tamanho_arquivo = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    if (tamanho_arquivo <= 0)
    {
        fclose(arquivo);
        puts("    <div class='error-box'>Erro: O arquivo <strong>horoscopo.xml</strong> está vazio ou inválido.</div>");
        puts("  </div></div></body></html>");
        return 1;
    }

    char *conteudo_xml = (char *)malloc((size_t)tamanho_arquivo + 1);
    if (!conteudo_xml)
    {
        fclose(arquivo);
        puts("    <div class='error-box'>Erro de memória ao carregar <strong>horoscopo.xml</strong>.</div>");
        puts("  </div></div></body></html>");
        return 1;
    }

    size_t lidos = fread(conteudo_xml, 1, (size_t)tamanho_arquivo, arquivo);
    conteudo_xml[lidos] = 0;
    fclose(arquivo);

    const char *cursor = conteudo_xml;
    int signo_encontrado = 0;
    int dia_encontrado = 0;

    while ((cursor = strstr(cursor, "<signo>")) != NULL)
    {
        const char *fim_signo = strstr(cursor, "</signo>");
        if (!fim_signo)
            break;

        char nome_xml[64] = {0};
        if (extrair_tag_xml_intervalo(cursor, fim_signo, "nome", nome_xml, sizeof(nome_xml)))
        {
            char nome_norm[64] = {0};
            normalizar_string(nome_xml, nome_norm, sizeof(nome_norm));

            if (strcmp(signo_busca, nome_norm) == 0)
            {
                signo_encontrado = 1;
                char exibicao[64] = {0};
                extrair_tag_xml_intervalo(cursor, fim_signo, "exibicao", exibicao, sizeof(exibicao));

                const char *simbolo = "&#10024;";
                const char *periodo = "";
                for (size_t i = 0; i < TOTAL_SIGNOS; i++)
                {
                    if (strcmp(nome_norm, LISTA_SIGNOS[i].id) == 0)
                    {
                        simbolo = LISTA_SIGNOS[i].simbolo;
                        periodo = LISTA_SIGNOS[i].periodo;
                        break;
                    }
                }

                const char *cursor_dia = cursor;
                while ((cursor_dia = strstr(cursor_dia, "<dia")) != NULL && cursor_dia < fim_signo)
                {
                    const char *fim_dia = strstr(cursor_dia, "</dia>");
                    if (!fim_dia || fim_dia > fim_signo)
                        break;

                    int num_dia = 0;
                    const char *fim_tag_abertura = strchr(cursor_dia, '>');
                    if (fim_tag_abertura && fim_tag_abertura < fim_dia)
                    {
                        const char *attr_num = strstr(cursor_dia, "numero=");
                        if (attr_num && attr_num < fim_tag_abertura)
                        {
                            attr_num += 7;
                            if (*attr_num == 34 || *attr_num == 39)
                                attr_num++;
                            num_dia = atoi(attr_num);
                        }
                    }
                    if (num_dia == 0)
                    {
                        char dia_str[16] = {0};
                        if (extrair_tag_xml_intervalo(cursor_dia, fim_dia, "numero", dia_str, sizeof(dia_str)))
                        {
                            num_dia = atoi(dia_str);
                        }
                    }

                    if (num_dia == dia_atual)
                    {
                        char previsao[MAX_TEXT] = {0};
                        char cor[64] = {0};
                        char numero_str[32] = {0};

                        extrair_tag_xml_intervalo(cursor_dia, fim_dia, "previsao", previsao, sizeof(previsao));
                        extrair_tag_xml_intervalo(cursor_dia, fim_dia, "cor", cor, sizeof(cor));
                        extrair_tag_xml_intervalo(cursor_dia, fim_dia, "numero_base", numero_str, sizeof(numero_str));

                        int num_base = atoi(numero_str);
                        int numero_sorte = ((num_base * 7 + data_hoje->tm_mday * 3 + (data_hoje->tm_mon + 1) * 5 + data_hoje->tm_year) % 60) + 1;
                        const char *hex_cor = obter_hex_cor(cor);

                        puts("    <div class='sign-header'>");
                        printf("      <div class='sign-icon'>%s</div>", simbolo);
                        fputc(10, stdout);
                        puts("      <div>");
                        printf("        <div class='sign-name'>%s</div>", exibicao[0] ? exibicao : nome_xml);
                        fputc(10, stdout);
                        if (periodo[0] != 0)
                        {
                            printf("        <div class='sign-period'>%s</div>", periodo);
                            fputc(10, stdout);
                        }
                        puts("      </div>");
                        puts("    </div>");

                        fputs("    <div class='previsao'>", stdout);
                        imprimir_html_escapado(previsao);
                        puts("</div>");

                        puts("    <div class='meta'>");
                        puts("      <div class='badge'>");
                        puts("        <span class='badge-label'>Nº da Sorte</span>");
                        printf("        <span class='badge-val'>&#127808; %02d</span>", numero_sorte);
                        fputc(10, stdout);
                        puts("      </div>");
                        puts("      <div class='badge'>");
                        puts("        <span class='badge-label'>Cor de Hoje</span>");
                        printf("        <span class='badge-val'><span class='color-dot' style='background:%s;'></span>", hex_cor);
                        imprimir_html_escapado(cor);
                        puts("</span>");
                        puts("      </div>");
                        puts("    </div>");

                        dia_encontrado = 1;
                        break;
                    }

                    cursor_dia = fim_dia + strlen("</dia>");
                }
                break;
            }
        }

        cursor = fim_signo + strlen("</signo>");
    }

    free(conteudo_xml);

    if (!signo_encontrado)
    {
        fputs("    <div class='error-box'>Signo <strong>'", stdout);
        imprimir_html_escapado(entrada_usuario);
        puts("'</strong> não encontrado.<br>Selecione um signo válido na lista acima.</div>");
    }
    else if (!dia_encontrado)
    {
        printf("    <div class='error-box'>Não foi encontrada previsão para o dia %d no signo informado.</div>", dia_atual);
        fputc(10, stdout);
    }

    puts("  </div>");
    puts("</div>");
    puts("</body>");
    puts("</html>");

    return 0;
}
