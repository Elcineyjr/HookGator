<h1 align="center">HookGator</h1>
<p align="center">
  <i>A hooking engine for 64 bit processes in a Windows environment</i>
  <br/><br/>
  
  <img width="240" src="https://user-images.githubusercontent.com/22310158/180611014-87bcdfac-cae9-46bf-8de1-285a82ed3b52.png" />
  <br/>
</p>

# Como funciona?

Monitora a criacao de um processo (especificado na linha de comando ao chama-lo) atraves do WMI - Windows Management Instrumentation. Essa eh uma interface de monitoracao do processo nativa do Windows.

Ao detectar a criacao de um novo processo, a *hooking engine* sera injetada nele.

# O que eh a hooking engine?

A *hooking engine* eh uma DLL cujo proposito e aplicar *inline patches* em funcoes nativas da API do Windows. Desta forma, alterando o comportamento da API.


# Como usar

```powershell
.\DLLInjector.exe -p <processo_alvo> <DLL_para_injetar>
```

O metodo de injecao utilizado foi o Inline Hook, tambem conhecido como Patch Hook. Essa maneira de injecao de codigo acontece ao sobrescrever instrucoes de uma chamada de API dentro do contexto do processo alvo, com a finalidade de redirecionar o fluxo de execucao do codigo.

As APIs do Windows sao uma maneira do usuario realizar chamadas a funcoes da ABI - *Application Binary Interface*. Atraves dessas chamadas, funcoes de leitura de arquivo, escrita de arquivo, funcoes de rede, e dentre varias outras, sao utilizadas por programadores. Sao equivalentes ao GLIBC em sistemas GNU/Linux.

Esse *hook*, ou gancho, eh aplicado atraves de um injetor de DLL. O papel do injetor eh criar uma thread no processo alvo, cuja finalidade eh carregar a DLL responsavel pela aplicacao do gancho (a *hook engine*).

# Onde utilizar

Patch Hooking eh uma forma de injecao de codigo extensivamente utilizada por solucoes de antivirus. Dentre outros motivos, pode ser utilizada para deteccao de chamadas de API suspeitas, visto que essa tecnica na interceptacao dessas chamadas, permitindo-nos homologar, processar, ou seja quaisquer outras atividades pertinentes.\
Tambem eh utilizada em cenarios ofensivos onde adversarios desejam um nivel mais avancado de discricao.

# Codigo

## C#

O codigo em C# eh responsavel pelo monitoramento, e subsequente injecao de DLL no processo alvo. Essa linguagem foi escolhida devido a sua facilidade de uso, permitindo codigos pouco verbosos e simples.

## C++

Ja o codigo feito em C++, foi dedicado a criacao da *Hook Engine*, isso porque, para que DLLs processos nativos do Windows sejam capazes de carregar uma DLL, ela precisa ser feita em uma linguagem de mais baixo nivel, conhecida pela terminologia da Microsoft como *unmanaged code*. Entao apenas DLL feitas em C++, C, ou Assembly. Embora seja possivel a criacao de DLLs utilizando C#, elas nao seriam capazes de ser carregadas por processos nativos, visto que elas sao consideradas *managed code*.