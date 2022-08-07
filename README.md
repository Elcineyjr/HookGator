<h1 align="center">HookGator</h1>
<p align="center">
  <i>Uma hooking engine para processadores 64 bit em um ambiente Windows</i>
  <br/><br/>
  
  <img width="240" src="https://user-images.githubusercontent.com/22310158/180611014-87bcdfac-cae9-46bf-8de1-285a82ed3b52.png" />
  <br/>
</p>

# Como funciona?

O HookGator monitora a criação de um processo (especificado na linha de comando ao chama-lo) através do WMI - *Windows Management Instrumentation*. Essa é uma interface de monitoramento do processo nativa do Windows. Ao detectar a criação de um novo processo, a *hooking engine* será injetada nele.

O método de injeção utilizado foi o *Inline Hook*, também conhecido como *Patch Hook*. Essa maneira de injeção de código acontece ao sobrescrever instruções de uma chamada de API dentro do contexto do processo alvo, com a finalidade de redirecionar o fluxo de execução do código.

As APIs do Windows são uma maneira do usuario realizar chamadas a funções da ABI - *Application Binary Interface*. Através dessas chamadas, funções de leitura de arquivo, escrita de arquivo, funções de rede, e dentre varias outras, são utilizadas por programadores. São equivalentes ao GLIBC em sistemas GNU/Linux.

Esse *hook*, ou gancho, é aplicado através de um injetor de DLL. O papel do injetor é criar uma *thread* no processo alvo, cuja finalidade é carregar a DLL responsável pela aplicação do gancho (a *hook engine*).

# O que é a hooking engine?

*Hooking* de funções da API do Windows é uma maneira antiga, porém ainda eficaz de interceptar chamadas à APIs do Windows, através da injeção de código no espaço de enderaçamento de um processo remoto alvo. Soluções de anti-vírus são notoriamente conhecidas por utilizar este método de monitoramento dinâmico de processos para detecção de chamadas suspeitas de API.

Porém, não apenas processos benignos podem se utilizar dessa forma de injeção. Famílias famosas de *malware*, como, por exemplo, Dridex, utilizavam-na como forma primária de subersão de mecanismos de defesas.

A *hooking engine* é uma DLL cujo propósito é aplicar *inline patches* em funções nativas da API do Windows. Desta forma, alterando o comportamento da API.


# Como usar

Em um terminal adminstrativo, execute

```powershell
.\DLLInjector.exe -p <processo_alvo> -d <DLL_para_injetar>
```

Onde: 

+ `p` é uma *string* que represente o nome do processo em memória;
+ `d` é o caminho absoluto para a DLL em disco.

Caso o programa não seja rodado com privilégio administrativo, ele irá requisitá-lo.

O produto vem com uma DLL de teste, e uma aplicação de teste. O código da aplicação de teste é como segue:

```C
#include <stdio.h>
#include <Windows.h>

void PrintBytes(char* pLocation) {
    int i;
    for (i = 0; i < 10; i++) {
        unsigned char c = (pLocation)[i];
        printf("%02x ", c);
    }
    printf("\n");
}


int main()
{
    // Avoid race conditions
    Sleep(2000);
    MessageBoxA(NULL, "test", "test", MB_OK);

    // Detect if the application itself is being hooked
    if (((PBYTE)MessageBoxA)[0] == 0xE9) {
        printf("[+] MessageBoxA is being hooked!\n");
        PrintBytes((char*)MessageBoxA);

        MessageBoxExA(NULL, "test", "test", MB_OK, 0);
    }

}
```

A aplicação também detecta quando ela mesma está sendo injetada.

# Onde utilizar

Como já dito, o HookGator utiliza *Patch Hooking*, que é uma forma de injeção de código extensivamente utilizada por soluções de antivirus. Dentre outros motivos, pode ser utilizada para detecção de chamadas de API suspeitas, visto que essa técnica consiste na interceptação dessas chamadas, permitindo-nos homologar, processar ou realizar quaisquer outras atividades pertinentes.

Esse método também é utilizado em cenários ofensivos onde adversários desejam um nivel mais avançado de discrição.

# Tecnologias usadas

## C#

O código em C# é responsável pelo monitoramento, e subsequente injeção de DLL no processo alvo. Essa linguagem foi escolhida devido a sua facilidade de uso, permitindo códigos pouco verbosos e simples.

## C++

Já o código feito em C++, foi dedicado a criação da *Hook Engine*, isso porque, para que processos nativos do Windows sejam capazes de carregar uma DLL, ela precisa ser feita em uma linguagem de mais baixo nivel, conhecida pela terminologia da Microsoft como *unmanaged code*. Então apenas DLL feitas em C++, C, ou Assembly. Embora seja possivel a criação de DLLs utilizando C#, elas não seriam capazes de ser carregadas por processos nativos, visto que elas são consideradas *managed code*.
