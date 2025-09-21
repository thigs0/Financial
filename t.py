# todo_app.py
import tkinter as tk
from tkinter import messagebox, ttk
import json
import os

class TodoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Lista de Tarefas")
        self.root.geometry("500x400")
        self.root.resizable(True, True)
        
        # Configuração de estilo
        self.style = ttk.Style()
        self.style.theme_use('clam')
        
        # Dados
        self.tasks = []
        self.data_file = "tasks.json"
        
        # Carregar tarefas salvas
        self.load_tasks()
        
        # Interface
        self.create_widgets()
        
        # Atualizar lista
        self.update_task_list()
    
    def create_widgets(self):
        # Frame principal
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configurar grid
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        
        # Entrada de nova tarefa
        ttk.Label(main_frame, text="Nova Tarefa:").grid(row=0, column=0, sticky=tk.W, pady=5)
        self.task_entry = ttk.Entry(main_frame, width=40)
        self.task_entry.grid(row=0, column=1, sticky=(tk.W, tk.E), pady=5, padx=5)
        self.task_entry.bind('<Return>', lambda e: self.add_task())
        
        # Botão adicionar
        add_btn = ttk.Button(main_frame, text="Adicionar", command=self.add_task)
        add_btn.grid(row=0, column=2, pady=5, padx=5)
        
        # Lista de tarefas
        self.task_listbox = tk.Listbox(main_frame, height=15, selectmode=tk.SINGLE)
        self.task_listbox.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=10, padx=5)
        
        # Barra de rolagem
        scrollbar = ttk.Scrollbar(main_frame, orient=tk.VERTICAL, command=self.task_listbox.yview)
        scrollbar.grid(row=1, column=3, sticky=(tk.N, tk.S), pady=10)
        self.task_listbox.configure(yscrollcommand=scrollbar.set)
        
        # Frame para botões
        btn_frame = ttk.Frame(main_frame)
        btn_frame.grid(row=2, column=0, columnspan=4, pady=10)
        
        # Botões de ação
        complete_btn = ttk.Button(btn_frame, text="Concluir", command=self.complete_task)
        complete_btn.pack(side=tk.LEFT, padx=5)
        
        delete_btn = ttk.Button(btn_frame, text="Excluir", command=self.delete_task)
        delete_btn.pack(side=tk.LEFT, padx=5)
        
        clear_btn = ttk.Button(btn_frame, text="Limpar Tudo", command=self.clear_tasks)
        clear_btn.pack(side=tk.LEFT, padx=5)
        
        # Configurar expansão
        main_frame.rowconfigure(1, weight=1)
        main_frame.columnconfigure(1, weight=1)
    
    def add_task(self):
        task_text = self.task_entry.get().strip()
        if task_text:
            self.tasks.append({"text": task_text, "completed": False})
            self.task_entry.delete(0, tk.END)
            self.update_task_list()
            self.save_tasks()
        else:
            messagebox.showwarning("Aviso", "Digite uma tarefa!")
    
    def complete_task(self):
        selected = self.task_listbox.curselection()
        if selected:
            index = selected[0]
            self.tasks[index]["completed"] = not self.tasks[index]["completed"]
            self.update_task_list()
            self.save_tasks()
        else:
            messagebox.showwarning("Aviso", "Selecione uma tarefa!")
    
    def delete_task(self):
        selected = self.task_listbox.curselection()
        if selected:
            index = selected[0]
            del self.tasks[index]
            self.update_task_list()
            self.save_tasks()
        else:
            messagebox.showwarning("Aviso", "Selecione uma tarefa!")
    
    def clear_tasks(self):
        if messagebox.askyesno("Confirmar", "Deseja limpar todas as tarefas?"):
            self.tasks = []
            self.update_task_list()
            self.save_tasks()
    
    def update_task_list(self):
        self.task_listbox.delete(0, tk.END)
        for task in self.tasks:
            text = task["text"]
            if task["completed"]:
                text = f"✓ {text}"
            self.task_listbox.insert(tk.END, text)
    
    def load_tasks(self):
        if os.path.exists(self.data_file):
            try:
                with open(self.data_file, 'r') as f:
                    self.tasks = json.load(f)
            except:
                self.tasks = []
    
    def save_tasks(self):
        with open(self.data_file, 'w') as f:
            json.dump(self.tasks, f)

def main():
    root = tk.Tk()
    app = TodoApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()
