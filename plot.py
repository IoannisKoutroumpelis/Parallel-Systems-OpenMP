import matplotlib.pyplot as plt
import re
import os

# --- Συναρτήσεις Parsing (Ίδιες με πριν) ---

def parse_q1(filepath):
    if not os.path.exists(filepath): return None
    threads, serial_times, parallel_times = [], [], []
    with open(filepath, 'r') as f:
        content = f.read()
    blocks = re.split(r'Verification: SUCCESS', content)
    for block in blocks:
        t_match = re.search(r'\((\d+) threads\)', block)
        s_match = re.search(r'Serial Execution Time: ([\d.]+) sec', block)
        p_match = re.search(r'Parallel Execution Time.*: ([\d.]+) sec', block)
        if t_match and s_match and p_match:
            threads.append(int(t_match.group(1)))
            serial_times.append(float(s_match.group(1)))
            parallel_times.append(float(p_match.group(1)))
    return threads, serial_times, parallel_times

def parse_q2(filepath):
    if not os.path.exists(filepath): return None
    data = []
    with open(filepath, 'r') as f:
        content = f.read()
    pattern = r"Matrix Size: (\d+)x\d+, Sparsity: (\d+)%, Threads: (\d+)\n.*?CSR Construction parallel: ([\d.]+) sec\n.*?CSR Construction serial: ([\d.]+) sec\n.*?SpMV CSR serial: ([\d.]+) sec\n.*?SpMV \(CSR\) parallel:\s+([\d.]+) sec\n.*?Dense Mult Time:\s+([\d.]+) sec"
    matches = re.findall(pattern, content)
    for m in matches:
        data.append({'size': int(m[0]), 'sparsity': int(m[1]), 'threads': int(m[2]), 
                     'csr_p': float(m[3]), 'csr_s': float(m[4]), 'spmv_s': float(m[5]), 
                     'spmv_p': float(m[6]), 'dense': float(m[7])})
    return data

def parse_q3(filepath):
    if not os.path.exists(filepath): return None
    data = []
    with open(filepath, 'r') as f:
        content = f.read()
    pattern = r"Sorting (\d+) elements \((Serial|Parallel)\) with (\d+) threads.*?\nSorted: YES\nExecution Time: ([\d.]+) seconds"
    matches = re.findall(pattern, content)
    for m in matches:
        data.append({'elements': int(m[0]), 'type': m[1], 'threads': int(m[2]), 'time': float(m[3])})
    return data

# --- Νέα Συνάρτηση για αποθήκευση σε φάκελο ---

def save_plot(fig, folder, filename, description):
    # Δημιουργία φακέλου αν δεν υπάρχει
    if not os.path.exists(folder):
        os.makedirs(folder)
    
    # Προσθήκη περιγραφής στο κάτω μέρος
    plt.figtext(0.1, 0.02, description, wrap=True, horizontalalignment='left', fontsize=9, color='#333333')
    plt.subplots_adjust(bottom=0.25) # Περισσότερος χώρος για το κείμενο
    
    # Αποθήκευση εικόνας
    filepath = os.path.join(folder, filename)
    fig.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close(fig)

# --- Κύριο Πρόγραμμα ---

def generate_plots():
    input_folder = "test"      # Ο φάκελος με τα .txt αρχεία
    output_folder = "results"  # Ο φάκελος που θα μπουν οι εικόνες
    
    # --- ΑΣΚΗΣΗ 1 ---
    res1 = parse_q1(os.path.join(input_folder, "q1_test.txt"))
    if res1:
        threads, serial, parallel = res1
        fig, ax = plt.subplots(figsize=(8, 6))
        ax.plot(threads, parallel, marker='o', color='blue', label='Parallel')
        ax.axhline(y=sum(serial)/len(serial), color='red', linestyle='--', label='Avg Serial')
        ax.set_title("Question 1 - Execution Time vs Threads")
        ax.set_xlabel("Threads")
        ax.set_ylabel("Time (seconds)")
        ax.legend()
        ax.grid(True)
        
        desc = ("Στο διάγραμμα παρατηρούμε τη μείωση του χρόνου εκτέλεσης καθώς αυξάνονται\n"
                "τα threads. Η παράλληλη εκτέλεση είναι σημαντικά ταχύτερη από τη σειριακή.")
        save_plot(fig, output_folder, "q1_performance.png", desc)

    # --- ΑΣΚΗΣΗ 2 ---
    res2 = parse_q2(os.path.join(input_folder, "q2_test.txt"))
    if res2:
        subset = [d for d in res2 if d['size'] == 1000 and d['threads'] == 4]
        subset.sort(key=lambda x: x['sparsity'])
        if subset:
            fig, ax = plt.subplots(figsize=(8, 6))
            sparsities = [d['sparsity'] for d in subset]
            ax.plot(sparsities, [d['spmv_s'] for d in subset], marker='s', label='SpMV Serial')
            ax.plot(sparsities, [d['spmv_p'] for d in subset], marker='o', label='SpMV Parallel')
            ax.set_title("Question 2 - Performance vs Sparsity")
            ax.set_xlabel("Sparsity (%)")
            ax.set_ylabel("Time (seconds)")
            ax.legend()
            ax.grid(True)
            
            desc = ("Εδώ βλέπουμε πώς η αραιότητα (sparsity) επηρεάζει την απόδοση.\n"
                    "Η μορφή CSR επωφελείται από υψηλότερη αραιότητα.")
            save_plot(fig, output_folder, "q2_sparsity.png", desc)

    # --- ΑΣΚΗΣΗ 3 ---
    res3 = parse_q3(os.path.join(input_folder, "q3_test.txt"))
    if res3:
        sizes = sorted(list(set(d['elements'] for d in res3)))
        for size in sizes:
            subset = [d for d in res3 if d['elements'] == size]
            subset.sort(key=lambda x: x['threads'])
            
            fig, ax = plt.subplots(figsize=(8, 6))
            ax.plot([d['threads'] for d in subset], [d['time'] for d in subset], marker='o', color='green')
            ax.set_title(f"Question 3 - Sorting {size:,} Elements")
            ax.set_xlabel("Threads")
            ax.set_ylabel("Time (seconds)")
            ax.grid(True)
            
            desc = (f"Ανάλυση ταξινόμησης για {size:,} στοιχεία. Παρατηρούμε τη σταθεροποίηση\n"
                    "της απόδοσης καθώς αυξάνεται το overhead διαχείρισης των νημάτων.")
            save_plot(fig, output_folder, f"q3_sort_{size}.png", desc)


if __name__ == "__main__":
    generate_plots()