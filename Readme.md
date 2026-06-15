# Leitor RFID - Sistema de Controle de Acesso

Este projeto implementa um sistema de controle de acesso utilizando RFID com Arduino e backend em Python como
trabalho final da disciplina de interfaces computacionais. O sistema é distribuído em três módulos principais: um leitor RFID, um gateway de processamento e um nó indicador de status.

---

## Arquitetura

O sistema é composto pelos seguintes módulos:


arduino/
gateway/ -> nó central de comunicação e decisão
no1/ -> leitor RFID (envia UID do cartão)
no2/ -> indicador visual (LED verde/vermelho)


### Fluxo de funcionamento

1. O nó **no1** realiza a leitura de um cartão RFID
2. O UID do cartão é enviado ao **gateway**
3. O **gateway** processa a validação do acesso
4. O resultado (permitido ou negado) é enviado ao **no2**
5. O nó **no2** indica o status:
   - LED verde → acesso permitido
   - LED vermelho → acesso negado

---

## Backend Python

O arquivo `leitor.py` é responsável por:

- Processar dados recebidos do gateway
- Validar acessos
- Definir lógica de permissão
- Integrar com possíveis interfaces ou sistemas externos

---

## Estrutura do projeto


arduino/
gateway/ -> comunicação central
no1/ -> leitor RFID
no2/ -> indicador LED

templates/ -> interface web (opcional)
leitor.py -> backend principal em Python
dependencias.txt -> dependências do projeto
venv/ -> ambiente virtual Python


---

## Requisitos

- Python 3.x
- Arduino IDE
- Bibliotecas listadas em `dependencias.txt`
- Módulo RFID (ex: MFRC522)

---

## Instalação

Crie e ative o ambiente virtual:

```bash
python -m venv venv
source venv/bin/activate

Instale as dependências:

pip install -r dependencias.txt
Execução
Faça upload dos códigos para os Arduinos:
no1
no2
gateway
Execute o backend Python:
python leitor.py
Funcionamento geral

O sistema funciona como um controle de acesso distribuído:

RFID é lido no nó de entrada
Gateway centraliza a decisão
Nó de saída visualiza o resultado
Possíveis melhorias
Banco de dados para registro de acessos
Interface web em tempo real
Sistema de usuários e permissões
Comunicação serial estruturada (JSON)
Logs com timestamp