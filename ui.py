import subprocess
import re
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

# Função para executar o código C e capturar a saída do terminal
def executar_codigo_c(limite_linhas=100, timeout=10):
    try:
        processo = subprocess.Popen(['./build/FreeRTOS-ubuntu'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        # Leitura em tempo real de stdout e stderr
        stdout_lines = []
        stderr_lines = []
        start_time = time.time()
        
        while True:
            stdout_line = processo.stdout.readline()
            stderr_line = processo.stderr.readline()
            
            if stdout_line:
                print(stdout_line.strip())  # Imprime a saída em tempo real
                stdout_lines.append(stdout_line)
                if len(stdout_lines) >= limite_linhas:
                    processo.terminate()
                    break
            if stderr_line:
                print(stderr_line.strip())  # Imprime a saída de erro em tempo real
                stderr_lines.append(stderr_line)
            
            # Verifica se o tempo limite foi atingido
            if time.time() - start_time > timeout:
                processo.terminate()
                break
            
            if not stdout_line and not stderr_line and processo.poll() is not None:
                break
        
        stdout = ''.join(stdout_lines)
        stderr = ''.join(stderr_lines)
        
        if stderr:
            print("Erro ao executar o código C:", stderr)
        
        return stdout
    except Exception as e:
        print("Erro ao executar o código C:", e)
        return ""

# Função para parsear a saída do terminal
def parsear_saida(saida):
    veiculos = {}
    cruzamentos = {}
    
    linhas = saida.split('\n')
    for linha in linhas:
        # Regex para capturar informações dos veículos
        match_veiculo = re.match(r'Veículo (\d+) se aproximando do cruzamento (\w) para mover (\w) com velocidade ([\d.]+) km/h. Tempo de percurso: (\d+) segundos', linha)
        if match_veiculo:
            id_veiculo = int(match_veiculo.group(1))
            cruzamento = match_veiculo.group(2)
            movimento = match_veiculo.group(3)
            velocidade = float(match_veiculo.group(4))
            tempo_percurso = int(match_veiculo.group(5))
            veiculos[id_veiculo] = {'cruzamento': cruzamento, 'movimento': movimento, 'velocidade': velocidade, 'tempo_percurso': tempo_percurso}
        
        # Regex para capturar informações dos cruzamentos
        match_cruzamento = re.match(r'Cruzamento (\w): Fase (\w+)', linha)
        if match_cruzamento:
            cruzamento = match_cruzamento.group(1)
            fase = match_cruzamento.group(2)
            cruzamentos[cruzamento] = fase
        
        # Regex para capturar movimento dos veículos
        match_movimento = re.match(r'Veículo (\d+) atravessou o cruzamento (\w) em (\w+)', linha)
        if match_movimento:
            id_veiculo = int(match_movimento.group(1))
            cruzamento = match_movimento.group(2)
            movimento = match_movimento.group(3)
            if id_veiculo in veiculos:
                veiculos[id_veiculo]['cruzamento'] = cruzamento
    
    return veiculos, cruzamentos

# Função de atualização para animação
def update(frame, veiculos, cruzamentos, scatter_veiculos, scatter_cruzamentos):
    plt.clf()
    for cruzamento, pos in posicoes_cruzamentos.items():
        plt.scatter(*pos, label=f'Cruzamento {cruzamento}', s=100)
    
    for id_veiculo, info in veiculos.items():
        pos = posicoes_cruzamentos[info['cruzamento']]
        plt.scatter(*pos, label=f'Veículo {id_veiculo}', s=50)
    
    plt.legend()
    plt.xlim(-100, DISTANCIA_CRUZAMENTO + 100)
    plt.ylim(-DISTANCIA_CRUZAMENTO - 100, 100)
    plt.title('Movimento dos Veículos')

# Posições dos cruzamentos para visualização
DISTANCIA_CRUZAMENTO = 500  # metros
posicoes_cruzamentos = {
    'A': (0, 0),
    'B': (DISTANCIA_CRUZAMENTO, 0),
    'C': (0, -DISTANCIA_CRUZAMENTO),
    'D': (DISTANCIA_CRUZAMENTO, -DISTANCIA_CRUZAMENTO)
}

# Captura a saída do terminal do código C
saida_terminal = executar_codigo_c(limite_linhas=100, timeout=10)

# Parseia a saída do terminal
veiculos, cruzamentos = parsear_saida(saida_terminal)

# Verifica se há dados para exibir
if not veiculos:
    print("Nenhum dado de veículo encontrado.")
if not cruzamentos:
    print("Nenhum dado de cruzamento encontrado.")

# Configuração da animação
fig = plt.figure()
scatter_veiculos = plt.scatter([], [])
scatter_cruzamentos = plt.scatter([], [])
ani = animation.FuncAnimation(fig, update, fargs=(veiculos, cruzamentos, scatter_veiculos, scatter_cruzamentos), frames=range(10), repeat=False)

try:
    plt.show()
except Exception as e:
    print("Erro ao exibir o gráfico:", e)