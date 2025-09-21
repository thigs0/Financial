# app_grafico.py
import tkinter as tk
from tkinter import messagebox

class AppAgenda:
    def __init__(self, root):
        self.root = root
        self.root.title("Agenda de Contatos")
        self.root.geometry("400x300")
        
        self.contatos = []
        
        # Widgets
        self.label_nome = tk.Label(root, text="Nome:")
        self.label_nome.pack(pady=5)
        
        self.entry_nome = tk.Entry(root, width=30)
        self.entry_nome.pack(pady=5)
        
        self.label_telefone = tk.Label(root, text="Telefone:")
        self.label_telefone.pack(pady=5)
        
        self.entry_telefone = tk.Entry(root, width=30)
        self.entry_telefone.pack(pady=5)
        
        self.btn_adicionar = tk.Button(root, text="Adicionar Contato", command=self.adicionar_contato)
        self.btn_adicionar.pack(pady=10)
        
        self.btn_listar = tk.Button(root, text="Listar Contatos", command=self.listar_contatos)
        self.btn_listar.pack(pady=5)
        
        self.text_area = tk.Text(root, height=10, width=40)
        self.text_area.pack(pady=10)
    
    def adicionar_contato(self):
        nome = self.entry_nome.get()
        telefone = self.entry_telefone.get()
        
        if nome and telefone:
            self.contatos.append({"nome": nome, "telefone": telefone})
            messagebox.showinfo("Sucesso", "Contato adicionado com sucesso!")
            self.entry_nome.delete(0, tk.END)
            self.entry_telefone.delete(0, tk.END)
        else:
            messagebox.showerror("Erro", "Preencha todos os campos!")
    
    def listar_contatos(self):
        self.text_area.delete(1.0, tk.END)
        if not self.contatos:
            self.text_area.insert(tk.END, "Nenhum contato cadastrado.")
        else:
            for i, contato in enumerate(self.contatos, 1):
                self.text_area.insert(tk.END, f"{i}. {contato['nome']} - {contato['telefone']}\n")

if __name__ == "__main__":
    root = tk.Tk()
    app = AppAgenda(root)
    root.mainloop()
