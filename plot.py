import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
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

# --- Συνάρτηση για προσθήκη σελίδας στο PDF ---

def add_plot_to_pdf(pdf, fig, title, description):
    # Προσθήκη περιγραφής στο κάτω μέρος του σχήματος
    plt.figtext(0.1, 0.02, description, wrap=True, horizontalalignment='left', fontsize=10, color='#333333')
    plt.subplots_adjust(bottom=0.2) # Αφήνουμε χώρο για το κείμενο
    pdf.savefig(fig)
    plt.close(fig)

# --- Κύριο Πρόγραμμα ---

def generate_report():
    folder = "test"
    pdf_filename = "Analysis_Report.pdf"
    
    with PdfPages(pdf_filename) as pdf:
        
        # --- ΑΣΚΗΣΗ 1 ---
        res1 = parse_q1(os.path.join(folder, "q1_test.txt"))
        if res1:
            threads, serial, parallel = res1
            fig, ax = plt.subplots(figsize=(8, 6))
            ax.plot(threads, parallel, marker='o', color='blue', label='Parallel')
            ax.axhline(y=sum(serial)/len(serial), color='red', linestyle='--', label='Avg Serial')
            ax.set_title("Question 1")
            ax.set_xlabel("Threads")
            ax.set_ylabel("Time (seconds)")
            ax.legend()
            ax.grid(True)
            
            desc = ("Στο διάγραμμα παρατηρούμε τη μείωση του χρόνου εκτέλεσης καθώς αυξάνονται\n"
                    "τα threads. Η παράλληλη εκτέλεση είναι σημαντικά ταχύτερη από τη σειριακή,\n"
                    "δείχνοντας καλό speedup.")
            add_plot_to_pdf(pdf, fig, "Q1", desc)

        # --- ΑΣΚΗΣΗ 2 ---
        res2 = parse_q2(os.path.join(folder, "q2_test.txt"))
        if res2:
            # Διάγραμμα για Sparsity
            subset = [d for d in res2 if d['size'] == 1000 and d['threads'] == 4]
            subset.sort(key=lambda x: x['sparsity'])
            if subset:
                fig, ax = plt.subplots(figsize=(8, 6))
                sparsities = [d['sparsity'] for d in subset]
                ax.plot(sparsities, [d['spmv_s'] for d in subset], marker='s', label='SpMV Serial')
                ax.plot(sparsities, [d['spmv_p'] for d in subset], marker='o', label='SpMV Parallel')
                ax.set_title("Question 2")
                ax.set_xlabel("Sparsity (%)")
                ax.set_ylabel("Time (seconds)")
                ax.legend()
                ax.grid(True)
                
                desc = ("Εδώ βλέπουμε πώς η αραιότητα (sparsity) του πίνακα επηρεάζει την απόδοση.\n"
                        "Όσο αυξάνεται το % των μηδενικών, ο χρόνος εκτέλεσης μειώνεται, καθώς η μορφή CSR\n"
                        "επεξεργάζεται μόνο τα μη μηδενικά στοιχεία.")
                add_plot_to_pdf(pdf, fig, "Q2", desc)

        # --- ΑΣΚΗΣΗ 3 ---
        res3 = parse_q3(os.path.join(folder, "q3_test.txt"))
        if res3:
            sizes = sorted(list(set(d['elements'] for d in res3)))
            for size in sizes:
                subset = [d for d in res3 if d['elements'] == size]
                subset.sort(key=lambda x: x['threads'])
                
                fig, ax = plt.subplots(figsize=(8, 6))
                ax.plot([d['threads'] for d in subset], [d['time'] for d in subset], marker='o', color='green')
                ax.set_title(f"Question 3")
                ax.set_xlabel("Threads")
                ax.set_ylabel("Time (seconds)")
                ax.grid(True)
                
                desc = (f"Ανάλυση ταξινόμησης για {size:,} στοιχεία. Παρατηρούμε ότι μετά από\n"
                        "έναν συγκεκριμένο αριθμό threads, η βελτίωση του χρόνου σταθεροποιείται λόγω\n"
                        "του overhead της διαχείρισης των νημάτων.")
                add_plot_to_pdf(pdf, fig, f"Q3_{size}", desc)


if __name__ == "__main__":
    generate_report()