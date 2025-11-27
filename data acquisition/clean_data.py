import re
from datetime import datetime
#TO REMOVE ANOMALOUS LOG LINES FROM THE FILE

NOME_FILE = "8accenspoiblocca.txt" #NAME FILE TO CLEAN, NEEDS TO STAY IN SAME DIRECTORY OT THE SCRIPT!!!!

def processa_file_log(nome_file_input, nome_file_output="output_pulito.txt"):
    
    righe_valide = []
    pattern = re.compile(r'"(\d{2}/\d{2}/\d{4} \d{2}:\d{2}:\d{2})" (-?\d+\.\d+) (-?\d+\.\d+) (-?\d+\.\d+)\s*')

    try:
        with open(nome_file_input, 'r', encoding='utf-8') as f_input:
            print(f"Lettura del file: '{nome_file_input}'...")
            for numero_riga, riga in enumerate(f_input, 1):
                match = pattern.match(riga.strip())

                if match:
                    data_ora_str = match.group(1)
                    valore1 = float(match.group(2))
                    valore2 = float(match.group(3))
                    valore3 = float(match.group(4))

                    righe_valide.append(f'"{data_ora_str}" {valore1} {valore2} {valore3}\n')
                else:
                    print(f"Riga {numero_riga}: Formato non corrispondente, riga ignorata: '{riga.strip()}'")

        if righe_valide:
            with open(nome_file_output, 'w', encoding='utf-8') as f_output:
                f_output.writelines(righe_valide)
            print(f"\nElaborazione completata. Righe valide salvate in: '{nome_file_output}'")
            print(f"Totale righe valide trovate: {len(righe_valide)}")
        else:
            print("\nNessuna riga valida trovata nel file.")

    except FileNotFoundError:
        print(f"Errore: Il file '{nome_file_input}' non è stato trovato.")
    except Exception as e:
        print(f"Si è verificato un errore inatteso: {e}")


if __name__ == "__main__":
    processa_file_log(NOME_FILE, NOME_FILE+"pulito.txt")

    
