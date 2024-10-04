import subprocess
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import re

# Função para ler a saída do programa C
def read_output():
    try:
        proc = subprocess.Popen(['./build/FreeRTOS-ubuntu'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    except Exception as e:
        print(f"Erro ao iniciar o programa C: {e}")
        return

    while True:
        line = proc.stdout.readline()
        if line:  # Verifica se há uma linha a ser lida
            print(f"Saída do programa C: {line.strip()}")  # Debug: Exibir a saída lida
            yield line.strip()
        else:
            break  # Sai do loop se não houver mais linhas

# Função para atualizar o gráfico
def update(frame):
    try:
        line = next(output_generator)
        match = re.findall(r'Cruzamento ([A-D]): (\d+) carros', line)
        for cruzamento, carros in match:
            dados_cruzamentos[cruzamento] = int(carros)
    except StopIteration:
        print("O programa C foi finalizado.")
        plt.close()
    except Exception as e:
        print(f"Erro: {e}")

    # Atualizar o gráfico
    bars.set_height(list(dados_cruzamentos.values()))

# Dicionário para armazenar os dados dos cruzamentos
dados_cruzamentos = {'A': 0, 'B': 0, 'C': 0, 'D': 0}

# Criar a figura do gráfico
fig, ax = plt.subplots()
bars = ax.bar(dados_cruzamentos.keys(), dados_cruzamentos.values(), color=['magenta', 'cyan', 'yellow', 'green'])
ax.set_ylim(0, 5)
ax.set_ylabel('Número de Carros')
ax.set_title('Carros nos Cruzamentos em Tempo Real')

# Inicializar o gerador de saída do programa C
output_generator = read_output()

# Criar a animação
ani = animation.FuncAnimation(fig, update, interval=1000)

plt.show()
