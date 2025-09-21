import pandas as pd
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import os

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.widgets import RangeSlider


class Pay_Fuzzy:
    def __init__(self, root):
        self.root = root
        self.root.title("Gráfico com Slider de Data + Categorias")
        self.root.geometry("1000x800")

        self.pf = pd.DataFrame()
        self.category_vars = {}  # para checkbuttons

        self.cumulative = None  # guardará DF acumulado para uso do slider

        self.create_widgets()

    def __read_nubank_csv(self, path) -> pd.DataFrame:
        def to(text):
            if not isinstance(text, str):
                return ''
            text = text.strip().lower()
            if 'débito' in text:
                parts = text.split('-')
                return parts[1].strip() if len(parts) > 1 else ''
            elif 'pix' in text:
                parts = text.split('-')
                return parts[1].strip() if len(parts) > 1 else ''
            elif 'fatura' in text:
                return 'nubank'
            elif 'celular' in text:
                parts = text.split('-')
                return parts[1].strip() if len(parts) > 1 else ''
            else:
                return ''

        df = pd.read_csv(path)
        out = pd.DataFrame(columns=['date','value','type_transfer','to','id','description'], index=range(len(df)))
        out.loc[:, 'date'] = pd.to_datetime(df.iloc[:,0], format='%d/%m/%Y')
        out.loc[:, 'value'] = df.iloc[:,1].astype(float)
        out.loc[:, 'id'] = df.iloc[:,2].astype(str)
        out.loc[:, 'type_transfer'] = df.iloc[:,3].str.split('-', expand=True, n=1).iloc[:,0]
        out.loc[:, 'description'] = df.iloc[:,3].str.split('-', expand=True, n=1).iloc[:,1]
        out.loc[:, 'to'] = df.iloc[:,3].apply(to)
        return out

    def create_widgets(self):
        main_frame = ttk.Frame(self.root, padding=10)
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # Botões de carregar/adicionar/salvar
        btn_load = ttk.Button(main_frame, text="Carregar hist.csv", command=self.load_hist_file)
        btn_load.grid(row=0, column=0, padx=5, pady=5)
        btn_add = ttk.Button(main_frame, text="Adicionar Nubank CSV", command=self.load_nubank_file)
        btn_add.grid(row=0, column=1, padx=5, pady=5)
        btn_save = ttk.Button(main_frame, text="Salvar hist.csv", command=self.save_pay_file)
        btn_save.grid(row=0, column=2, padx=5, pady=5)

        # Categorias com Checkbuttons
        ttk.Label(main_frame, text="Categorias:").grid(row=1, column=0, sticky=tk.W, pady=(10,0))
        self.cat_frame = ttk.Frame(main_frame, borderwidth=1, relief=tk.SUNKEN)
        self.cat_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W,tk.E), padx=5)

        self.canvas_cat = tk.Canvas(self.cat_frame, height=150)
        self.scrollbar_cat = ttk.Scrollbar(self.cat_frame, orient=tk.VERTICAL, command=self.canvas_cat.yview)
        self.inner_cat = ttk.Frame(self.canvas_cat)

        self.inner_cat.bind(
            "<Configure>",
            lambda e: self.canvas_cat.configure(scrollregion=self.canvas_cat.bbox("all"))
        )
        self.canvas_cat.create_window((0,0), window=self.inner_cat, anchor='nw')
        self.canvas_cat.configure(yscrollcommand=self.scrollbar_cat.set)

        self.canvas_cat.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.scrollbar_cat.pack(side=tk.RIGHT, fill=tk.Y)

        btn_sel_all = ttk.Button(main_frame, text="Selecionar todos", command=lambda: self.set_all_categories(True))
        btn_sel_all.grid(row=3, column=0, pady=5, padx=5)
        btn_clear_all = ttk.Button(main_frame, text="Desmarcar todos", command=lambda: self.set_all_categories(False))
        btn_clear_all.grid(row=3, column=1, pady=5, padx=5)

        # Botão para atualizar gráfico
        btn_update = ttk.Button(main_frame, text="Atualizar Gráfico", command=self.update_graph_clicked)
        btn_update.grid(row=0, column=3, padx=5, pady=5)

        # Frame para o gráfico
        self.graph_frame = ttk.Frame(main_frame)
        self.graph_frame.grid(row=4, column=0, columnspan=4, pady=(20,0), sticky=(tk.W,tk.E,tk.N,tk.S))

        # Configurar redimencionamento
        main_frame.columnconfigure(3, weight=1)
        main_frame.rowconfigure(4, weight=1)

    def load_hist_file(self):
        if os.path.exists('hist.csv'):
            self.pf = pd.read_csv('hist.csv', parse_dates=['date'])
            messagebox.showinfo("Sucesso", f"Carregado {len(self.pf)} registros.")
            self.update_category_checks()
        else:
            messagebox.showwarning("Aviso", "hist.csv não encontrado.")

    def load_nubank_file(self):
        path = filedialog.askopenfilename(title="Selecionar CSV Nubank", filetypes=[("CSV","*.csv")])
        if path:
            new_df = self.__read_nubank_csv(path)
            self.pf = pd.concat([self.pf, new_df], ignore_index=True)
            messagebox.showinfo("Sucesso", f"{os.path.basename(path)} adicionado.")
            self.update_category_checks()

    def save_pay_file(self):
        if self.pf.empty:
            messagebox.showwarning("Aviso", "Nenhum dado para salvar.")
            return
        self.pf.to_csv('hist.csv', index=False)
        messagebox.showinfo("Sucesso", "Arquivo salvo.")

    def update_category_checks(self):
        # Limpar existentes
        for w in self.inner_cat.winfo_children():
            w.destroy()
        self.category_vars.clear()

        if 'to' not in self.pf.columns:
            return

        cats = sorted(self.pf['to'].dropna().unique().tolist())
        for i, cat in enumerate(cats):
            var = tk.BooleanVar(value=False)
            chk = ttk.Checkbutton(self.inner_cat, text=cat, variable=var)
            chk.grid(row=i//2, column=i%2, sticky=tk.W, padx=5, pady=2)
            self.category_vars[cat] = var

    def set_all_categories(self, state: bool):
        for var in self.category_vars.values():
            var.set(state)

    def get_selected_categories(self):
        return [cat for cat,var in self.category_vars.items() if var.get()]

    def update_graph_clicked(self):
        if self.pf.empty:
            messagebox.showwarning("Aviso", "Nenhum dado carregado.")
            return

        selected = self.get_selected_categories()
        if selected:
            df_f = self.pf[self.pf['to'].isin(selected)]
        else:
            df_f = self.pf.copy()

        df_f = df_f.sort_values('date')
        daily = df_f.groupby('date')['value'].sum().sort_index()
        cumulative = daily.cumsum().reset_index()
        cumulative.columns = ['date','value_cumulative']
        self.cumulative = cumulative

        # limpar gráfico anterior
        for widget in self.graph_frame.winfo_children():
            widget.destroy()

        # Criar figura e eixo
        fig = plt.Figure(figsize=(8,5))
        ax = fig.add_subplot(111)
        line, = ax.plot(cumulative['date'], cumulative['value_cumulative'], marker='o', linestyle='-')
        ax.set_title(f"Acumulado — Categorias: {', '.join(selected) if selected else 'Todos'}")
        ax.set_xlabel("Data")
        ax.set_ylabel("Valor acumulado")
        ax.grid(True)
        ax.xaxis.set_major_formatter(mdates.DateFormatter("%Y-%m-%d"))
        fig.autofmt_xdate()

        fig.subplots_adjust(bottom=0.25)  # espaço para slider

        canvas = FigureCanvasTkAgg(fig, master=self.graph_frame)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Slider de intervalo de datas sobre o eixo de índices do DataFrame acumulado
        n = len(cumulative)
        if n < 2:
            return

        slider_ax = fig.add_axes([0.15, 0.1, 0.7, 0.03])
        range_slider = RangeSlider(
            slider_ax,
            "Intervalo",
            valmin=0,
            valmax=n - 1,
            valinit=(0, n - 1),
            valstep=1
        )

        def update(val):
            start_idx, end_idx = int(range_slider.val[0]), int(range_slider.val[1])
            sub = self.cumulative.iloc[start_idx:end_idx + 1]
            if sub.empty:
                return
            line.set_data(sub['date'], sub['value_cumulative'])
            ax.set_xlim(sub['date'].iloc[0], sub['date'].iloc[-1])
            ax.set_ylim(sub['value_cumulative'].min(), sub['value_cumulative'].max())
            fig.canvas.draw_idle()

        range_slider.on_changed(update)


def main():
    root = tk.Tk()
    app = Pay_Fuzzy(root)
    root.mainloop()


if __name__ == "__main__":
    main()
