import io
import json
import os
import sys
from urllib.parse import parse_qs
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

query_string = os.environ.get('QUERY_STRING', '')
arguments = parse_qs(query_string)

filename_1 = os.path.join(Path(__file__).resolve().parent, 'estados.geojson')
filename_2 = os.path.join(Path(__file__).resolve().parent, 'dados-estados-populacao.json')

with open(filename_1, 'r', encoding='utf-8') as f:
    geojson_data = json.load(f)
states_list = []
for feature in geojson_data['features']:
    props = feature['properties']
    states_list.append({
        'codigo': props['codigo'],
        'local': props['local']
    })
df_states = pd.DataFrame(states_list)

with open(filename_2, 'r', encoding='utf-8') as f:
    states_data = json.load(f)
df_data = pd.DataFrame(states_data)
df_data_filtered = df_data[df_data['D2C'] == int(arguments.get('D2C')[0])].copy()
df = pd.merge(
    df_states, 
    df_data_filtered, 
    left_on='codigo', 
    right_on='D1C'
)
df = df.sort_values('local')

plt.figure(figsize=(11.5, 6.5))
if (int(arguments.get('D2C')[0]) == 10605):
    bars = plt.bar(df['local'], df['V'], color='skyblue')
else:
    bars = plt.bar(df['local'], df['V'], color='orange')    
plt.xticks(rotation=45, ha='right')
plt.grid(axis='y', linestyle='--', alpha=0.7)
for bar in bars:
    yval = bar.get_height()
    if (int(arguments.get('D2C')[0]) == 10605):
        plt.text(bar.get_x() + bar.get_width()/2, yval, f'{yval:.2f}', va='bottom', ha='center', fontsize=9)
    else:
        plt.text(bar.get_x() + bar.get_width()/2, yval, f'{yval:.0f}', va='bottom', ha='center', fontsize=9)
plt.tight_layout()

bf = io.BytesIO()
plt.savefig(bf, format='png')
data = bf.getvalue() 
size = len(data) 

try:
    sys.stdout.write("Content-Type: image/png\n")
    sys.stdout.write(f"Content-Length: {size}\n\n")
    sys.stdout.flush()
    sys.stdout.buffer.write(data)
    sys.stdout.buffer.flush()
except Exception as e:
    sys.stdout.write("Status: 500 Internal Server Error\n")
    sys.stdout.write("Content-Type: text/plain\n\n")
    sys.stdout.write(f"Error: {str(e)}")

bf.close()
plt.close()
