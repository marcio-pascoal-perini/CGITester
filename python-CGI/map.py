import json
import os
import sys
import folium
import pandas as pd
from pathlib import Path
from urllib.parse import parse_qs

query_string = os.environ.get('QUERY_STRING', '')
arguments = parse_qs(query_string)

state_geo = os.path.join(Path(__file__).resolve().parent, 'estados.geojson')

state_data = pd.read_json(
    path_or_buf = os.path.join(Path(__file__).resolve().parent, 'dados-estados-populacao.json'),
    encoding='utf-8'
)

# state_data = state_data.loc[
#     (state_data['MC'] == int(arguments.get('MC')[0])) &
#     (state_data['D2C'] == int(arguments.get('D2C')[0])) 
# ]

state_data = state_data.loc[(state_data['D2C'] == int(arguments.get('D2C')[0]))]

map = folium.Map(location=[-16.1237611, -59.9219642], zoom_start=4)
folium.Choropleth(
    geo_data=state_geo,
    name='Estados do Brasil',
    data=state_data,
    columns=['D1C', 'V'],
    key_on='feature.properties.codigo',
    fill_color='Reds',
    fill_opacity=0.7,
    line_opacity=1,
    line_color='#FFFFFF',
    line_weight=2,
    # legend_name='Variação acumulada de 12 meses - Receita nominal (Fev 2019)',
    highlight=True,
).add_to(map)

with open(state_geo, 'r', encoding='utf-8') as f:
    geojson_dict = json.load(f)
v_mapping = state_data.set_index('D1C')['V'].to_dict()
for feature in geojson_dict['features']:
    state_code = feature['properties']['codigo']
    feature['properties']['V'] = v_mapping.get(state_code, 'Sem dados')

folium.GeoJson(
    geojson_dict,
    style_function=lambda x: {'fillColor': '#ffffff00', 'color': '#ffffff00'},
    tooltip=folium.GeoJsonTooltip(
        fields=['codigo', 'local', 'V'],
        aliases=['Código:', 'Local:', 'Valor:'],
        localize=True
    )
).add_to(map)

# map.save('brazil.html')

html = map.get_root().render()
size = len(html.encode('utf-8'))
size += 500

try:
   sys.stdout.write("Content-Type: text/html; charset=utf-8\n")
   sys.stdout.write(f"Content-Length: {str(size)}\n\n")
   sys.stdout.write(html)
except Exception as e:
   sys.stdout.write("Status: 500 Internal Server Error\n")
   sys.stdout.write("Content-Type: text/plain\n\n")
   sys.stdout.write(f"Error: {str(e)}")
