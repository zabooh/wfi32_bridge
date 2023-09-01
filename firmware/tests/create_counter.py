def write_counters_to_ascii_file(counters, filename):
    with open(filename, 'a') as file:  # Verwende 'a' für Append-Modus
        for counter in counters:
            file.write(str(counter) + '\n')  # Schreibe den Zählerwert und füge eine neue Zeile hinzu

def main():
    target_value = 100000  # Hier kannst du die gewünschte Zielzahl angeben
    counters = list(range(target_value + 1))  # Erzeuge eine Liste von Zählern von 0 bis target_value

    write_counters_to_ascii_file(counters, 'counter.txt')

if __name__ == "__main__":
    main()