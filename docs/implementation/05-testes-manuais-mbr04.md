# Testes manuais — MBR-00 a MBR-04

Candidato de implementação para **Raspberry Pi Pico 2 W (RP2350)** com **Waveshare Pico-LCD-1.3 / ST7789 240×240**. Nenhum teste físico foi marcado PASS pelo executor. O operador autorizou adiar as verificações físicas até esta entrega.

## Arquivos e instalação

- `mouse_bridge_remapper_qualification.uf2`: use primeiro, para executar os cenários abaixo. Menu explícito de qualificação; dados simulados somente em RAM; sem Bluetooth e sem gravação de preferências.
- `mouse_bridge_remapper.uf2`: firmware de produção até MBR-04. Abre SEARCHING FIRST MOUSE, tem HAT/LCD e identidade USB real; ainda não descobre nem conecta mouses Bluetooth. Essa implementação começa no MBR-05.

Com a placa desconectada, mantenha BOOTSEL pressionado, conecte por USB e solte BOOTSEL. Copie **um** arquivo `.uf2` para a unidade de boot que aparecer. A placa reinicia automaticamente. Para trocar de firmware, repita o procedimento. Não é necessário apagar a flash inteira.

Use cabo USB de dados. Encaixe o HAT com a placa desligada. A placa alvo é Pico 2 W; este candidato não foi preparado para outras placas RP2350 com pinagem diferente.

## Controles exclusivos da qualificação

Menu inicial: JOY UP/DOWN seleciona e JOY PRESS acessa, sempre ao soltar.

- **SCREEN GALLERY**: A avança, B retrocede, Y volta ao menu, X abre a tela atual em navegação interativa. A galeria exibe literalmente as telas do produto; estes controles externos da galeria não pertencem ao firmware de produção.
- **INTERACTIVE UI**: começa em FIRST MOUSE CONNECTED com dois mouses fictícios em RAM. Controles iguais aos do produto. Aplicar/remover recebe confirmação simulada após 500 ms. Para voltar ao menu inicial, desconecte e reconecte o USB. Não há atalho oculto adicionado à navegação do produto.
- Ao abrir SEARCHING FIRST MOUSE pela galeria usando X, um mouse fictício é confirmado após aproximadamente 3,5 segundos. As buscas Saved e Pair New ficam sem vencedor, permitindo medir seus timeouts.
- **USB OUTPUT TESTS**: relatórios USB reais produzidos pelo HAT, somente após seleção explícita de um teste. B retorna e libera todas as saídas. Não são relatórios recebidos por Bluetooth.

Reiniciar sempre restaura os dados simulados; isso não é teste de persistência.

## Cenários enumerados

1. **Inicialização de qualificação.** Instale o UF2 de qualificação. Esperado: título `MBR-04 QUALIFICATION`, aviso `RAM FIXTURES ONLY`, três opções e `NO BLUETOOTH IN MBR04`. Sem tela branca permanente ou rotação incorreta.
2. **Inventário das 30 telas.** Entre em SCREEN GALLERY e pressione/solte A 30 vezes. Esperado: percorre as 30 telas na ordem de `docs/manual/06-screen-reference.md` e retorna a SEARCHING FIRST MOUSE; B percorre em sentido inverso. Nenhuma tela Keyboard, Composite ou contagem de mouses simultaneamente conectados.
3. **Pixels e legibilidade.** Observe títulos, linhas longas, última linha e margens nas 30 telas. Esperado: nenhum caractere cortado, nenhuma linha sobreposta; título em x=7/y=8; glifos 10×14; avanço horizontal 11; primeiro corpo y=39 e última dica y=214. Faixa inferior magenta-escura começa 11 pixels antes da primeira dica. Telas didáticas têm fundo magenta-escuro completo.
4. **Cores das dicas.** Sem pressionar controles, examine todas as dicas inferiores: palavras inteiras em cinza-claro RGB565 `C618`, não amarelas. Títulos magenta; texto explicativo amarelo-claro; seleção branca; estado atual/correto ciano. Em Help, frases explicativas contendo `KEY B` continuam no corpo amarelo-claro; somente `ANY KEY: BACK` é dica cinza-claro.
5. **Colunas didáticas.** Na galeria SEARCHING FIRST MOUSE, segure cada direção, JOY PRESS e A/B/X/Y individualmente antes de soltar. Esperado: o rótulo certo fica branco enquanto pressionado. JOY UP coluna 8; JOY da esquerda/centro/direita colunas 3/10/17; LEFT/PRESS/RIGHT 3/9/16; JOY DOWN 7. A/X e B/Y nas colunas 2/16. A/B mudam a tela somente ao soltar por serem controles da galeria.
6. **Aprender teclas e bloqueio didático.** Reinicie, entre em INTERACTIVE UI. Joystick e A só dão feedback; B apaga a apresentação ao soltar; A e joystick não desbloqueiam; X desbloqueia e permanece na mesma tela; Y abre HOME ao soltar. Em HOME, escolha LEARN THE KEYS e repita. LOCK SCREEN começa na coluna 1; AND UNLOCK na 2; OPEN HOME -> KEY Y na 3.
7. **Debounce e ação ao soltar.** Em HOME, segure DOWN por dois segundos: não deve navegar antes de soltar; ao soltar avança uma opção apenas. Repita 20 cliques separados e confira que não há duplicações. Depois percorra o início/fim da lista e confirme wrap.
8. **HOME conectado.** Na navegação interativa, HOME deve ter título LOGITECH LIFT e opções nesta ordem: PAIR NEW MOUSE, REMAPPED TO ESCAPE, SAVED DEVICES, LEARN THE KEYS. Selecione a segunda opção: abre MOUSE OPTIONS. Volte com JOY LEFT.
9. **Help consome o evento.** Em HOME abra Help com X; pressione/solte B: somente fecha Help. Repita com A, X, Y e JOY PRESS, reabrindo Help a cada vez. Nenhum deve executar também a ação da tela subjacente ou bloquear a tela.
10. **Pair New e Help literal.** Em HOME escolha PAIR NEW MOUSE. Abra Help com X e compare as seis linhas do texto com a documentação, incluindo `DEVICE, FIRST UNPLUG` e `KEY B TO BACK UNTIL`. Volte e cancele com B antes de 15 segundos: o mouse fictício original continua conectado em HOME, sem ser removido do cadastro.
11. **Timeout Pair New e bloqueio comum.** Entre novamente em Pair New, bloqueie com Y e aguarde pelo menos 15 segundos. Uma interação completa de qualquer tecla desbloqueia e é consumida. Esperado: NO NEW MOUSE OUTSIDE...; A reinicia busca; B retorna a HOME ainda conectado. X exibe DEVICE NOT FOUND HELP com o mesmo texto exato de reconexão salva.
12. **Busca salva de 8 segundos.** Reinicie, na galeria avance até HOME SEARCHING e use X para entrar no fluxo. Esperado: após 8 segundos, DEVICE NOT FOUND; A reinicia outra busca de 8 segundos; B durante busca cancela. Abra Help durante busca, aguarde o timeout e feche Help: retorna ao estado de tentativa expirada, sem iniciar outra busca acidentalmente. Compare o texto HOME SEARCHING HELP com a tabela canônica.
13. **Perfis, confirmação e prioridade de cor.** Em INTERACTIVE UI -> HOME -> remap -> MOUSE OPTIONS, o perfil Escape atual aparece ciano quando não selecionado e branco quando selecionado. Abra Standard e pressione A: somente após a confirmação simulada de 500 ms aparece STANDARD REMAP ACTIVE, com corpo ciano e dicas cinza. Na tela Escape não aplicada, Y não bloqueia; na Escape aplicada, JOY LEFT leva diretamente a HOME.
14. **Custom e nomes longos.** Abra CUSTOM REMAP, escolha LEFT, selecione ESCAPE e aplique com A. Após confirmação, EDIT CUSTOM REMAP mostra LEFT IS ESCAPE imediatamente. Em SAVED DEVICES, navegue ao segundo mouse: nome mostra apenas os primeiros 21 caracteres suportados, sem reticências, STATUS: DISCONNECTED e cor de corpo; a página do primeiro mouse é a única com nome ciano e STATUS: CONNECTED.
15. **Remoção confirmada em RAM.** Em SAVED DEVICES, selecione o segundo mouse, JOY PRESS e A. Só após confirmação a página desaparece, sem desconectar o primeiro mouse. Remova o último mouse: entra em SEARCHING FIRST MOUSE. Este teste verifica a navegação; limpeza real de credenciais e persistência pertence a gates posteriores.
16. **Descritores USB reais.** No Linux, execute `lsusb -d cafe:4011 -v` (use sudo se precisar ler todos os descritores); no Windows use uma ferramenta de inspeção USB. Esperado: VID CAFE, PID 4011, bcdDevice 1.00, fabricante `tiagooliveirajs`, produto `Mouse Bridge Remapper`, sem serial; exatamente duas interfaces HID: interface 0 Mouse e interface 1 Keyboard sintético. Sem CDC/porta serial. Interfaces usam Report Protocol, não anunciam Boot Protocol.
17. **Cinco botões USB.** Reinicie -> USB OUTPUT TESTS -> FIVE MOUSE BUTTONS. Em um visualizador de eventos de entrada, mantenha cada controle pressionado e solte: UP=esquerdo, DOWN=direito, PRESS=meio, LEFT=Backward, RIGHT=Forward. Esperado: down/hold/up correto, sem botão preso. Combine um botão físico A/B não mapeado com joystick e confirme que não cria outro botão de mouse; B sai e libera tudo.
18. **Movimento USB.** USB OUTPUT TESTS -> X / Y MOVEMENT. Segure direções do joystick; esperado: movimento no eixo e sentido correspondente, inclusive diagonais quando o HAT permitir. Solte: movimento para. B volta ao menu sem movimento residual.
19. **Roda e pan USB.** USB OUTPUT TESTS -> WHEEL / PAN. UP/DOWN geram roda vertical; LEFT/RIGHT geram pan horizontal. Confirme em visualizador de eventos ou aplicativo com rolagem horizontal suportada. Soltar para a rolagem; B sai sem eventos residuais.
20. **Escape USB pressionado/solto.** USB OUTPUT TESTS -> ESCAPE HOLD. Em um visualizador de teclado, segure A: somente Escape (`0x29`) é pressionado; solte A: Escape é liberado. Repetição automática durante hold pode ser produzida pelo sistema operacional. Nenhuma outra tecla/modificador. B retorna e libera Escape mesmo se A estava segurado.
21. **Estabilidade da enumeração.** Com monitor USB/eventos aberto, navegue, abra Help, bloqueie/desbloqueie e execute os quatro testes USB. Esperado: nenhuma desconexão/reconexão USB causada pela interface. Desconectar/reconectar fisicamente enumera o mesmo produto; suspenda/retome o host e verifique que não sobra botão/Escape preso.
22. **Firmware de produção.** Instale `mouse_bridge_remapper.uf2`. Esperado: SEARCHING FIRST MOUSE permanente, ciclos internos de 8 segundos, todos os controles apenas didáticos, sem conexão fictícia, sem menu de qualificação. Identidade USB igual ao cenário 16, saídas em repouso. Mouse Bluetooth ainda não conecta neste gate; isso não é falha do MBR-04.

## Registro de resultados

Informe: nome exato do UF2, SHA-256 do manifesto, modelo da placa/HAT, sistema operacional, número do cenário, PASS/FAIL e comportamento observado. Para falhas de pixels/cores, anexe foto da tela inteira. Falha física mantém a validação correspondente pendente; não exige executar BLE/remapeamento real antes do MBR-05/06.
