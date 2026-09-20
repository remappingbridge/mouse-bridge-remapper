# Testes físicos — candidato MBR-05

MBR-00 a MBR-04 foram aceitos pelo operador em 2026-09-20. Estes testes se referem
à nova produção `mouse_bridge_remapper.uf2`; use o SHA-256 registrado na evidência
do candidato. A imagem `_qualification.uf2` é apenas para regressão dos fixtures,
não conecta Bluetooth. O hardware alvo é Pico 2 W + Waveshare Pico-LCD-1.3.

1. **USB e início:** grave a produção e conecte ao computador. Deve aparecer o
   mesmo Mouse Bridge Remapper (CAFE:4011), Mouse + Escape HID, sem CDC. A tela
   searching-first deve ter magenta em todo o fundo, inclusive quatro bordas.
2. **Pareamento novo:** ative o modo de pareamento de um mouse BLE HOGP genérico
   em um canal disponível. Ao ficar pronto, deve aparecer first-mouse-connected,
   também todo magenta. Não é necessário backend Logitech. Teclado BLE não deve
   tornar-se Mouse conectado nem gerar entrada USB.
3. **Título e opções HOME:** pressione/solte Y. Para LIFT, o título é LIFT MOUSE;
   MOUSE GENERIC permanece assim. XPTO ULTRA 2714 tem 14 caracteres e vira
   XPTO ULTRA 2714 MOUSE. Nome maior que 15 é cortado antes do sufixo. Confira o
   nome disponível no seu mouse; nomes artificiais são cobertos nos testes host.
   Percorra as opções: selecionada branca, demais cinza-claro, inclusive a opção
   do remapper. Searching/retry usam a mesma regra, sem ciano nas opções.
4. **Movimento e botões:** teste X/Y nas duas direções, Left/Right/Middle e
   Forward/Backward disponíveis. Arraste mantendo cada botão pressionado e solte.
   Teste cliques simultâneos, roda vertical e pan horizontal se o mouse o oferece.
5. **UI durante uso:** navegue para Learn the Keys, confira todo o fundo magenta,
   bloqueie com B, mova/clique/role o mouse com a tela apagada, desbloqueie com X.
   O mouse continua funcionando; não há reconexão ou nova enumeração USB.
6. **Desconexão pressionada:** volte à HOME, mantenha um botão em arraste e desligue
   o mouse. O arraste deve terminar; nenhum botão pode ficar preso. HOME entra em
   busca de salvos por 8 s e depois DEVICE NOT FOUND se ele continuar desligado.
7. **Reconexão na mesma inicialização:** ligue o mouse durante a busca. Se já
   expirou, use A em HOME-retry. O mesmo mouse deve reconectar e voltar a mover,
   clicar e rolar. Repita desligar/religar mantendo botão; não deve haver evento
   fantasma da sessão anterior.
8. **Pair New com mouse saudável:** abra Pair New; o atual permanece utilizável
   durante os 15 s e após cancelar/expirar. Substituição por outro mouse será
   qualificada no MBR-07; não é critério de sucesso desta imagem.
9. **Regressão de MBR-03/04:** use a nova imagem qualification, se necessário,
   para as 30 telas/HAT, títulos/cores e saída USB Mouse/Escape do roteiro MBR-04.
   Restaure a produção para concluir. A qualification continua sem rádio.

Informe PASS/FAIL por cenário, modelo do mouse, sistema do computador e o hash do
UF2 utilizado. Ausência de pan/botão no hardware é N/A, não um PASS simulado.
Persistência do registro após reboot, perfis, HID++ e remoção real são etapas
posteriores; esta imagem não anuncia sucesso para efeitos ainda não implementados.
