#SOME LOG FILES USES A WRONG SEPARATOR (,), TO FIX THIS AND UNIFORM ALL THE LOGS

import re

NOME_FILE ="3medio.txt"  #NAME FILE TO CLEAN, NEEDS TO BE IN SAME DIRECTORY SCRIPT!!!!

def converti_formato_log(nome_file_input, nome_file_output="output_convertito.txt"):
    """

    Legge un file di log con un formato specifico e lo converte in un nuovo formato.
    
    """
    righe_convertite = []
    pattern_input = re.compile(r'"(\d{2}/\d{2}/\d{4} \d{2}:\d{2}:\d{2})",(-?\d+\.\d+) (-?\d+\.\d+) (-?\d+\.\d+)\s*')

    try:
        with open(nome_file_input, 'r', encoding='utf-8') as f_input:
            print(f"Lettura del file: '{nome_file_input}' per la conversione...")
            for numero_riga, riga in enumerate(f_input, 1):
                match = pattern_input.match(riga.strip())

                if match:
                    data_ora_str = match.group(1)
                    valore1 = float(match.group(2))
                    valore2 = float(match.group(3))
                    valore3 = float(match.group(4))

                    nuova_riga = f'"{data_ora_str}" {valore1} {valore2} {valore3}\n'
                    righe_convertite.append(nuova_riga)
                else:
                    print(f"Riga {numero_riga}: Formato non corrispondente all'input atteso, riga ignorata: '{riga.strip()}'")

        if righe_convertite:
            with open(nome_file_output, 'w', encoding='utf-8') as f_output:
                f_output.writelines(righe_convertite)
            print(f"\nConversione completata. Righe convertite salvate in: '{nome_file_output}'")
            print(f"Totale righe convertite: {len(righe_convertite)}")
        else:
            print("\nNessuna riga valida trovata nel file di input per la conversione.")

    except FileNotFoundError:
        print(f"Errore: Il file '{nome_file_input}' non è stato trovato.")
    except Exception as e:
        print(f"Si è verificato un errore inatteso durante la conversione: {e}")

if __name__ == "__main__":
    
   
    converti_formato_log(NOME_FILE, NOME_FILE+'pulito.txt')
