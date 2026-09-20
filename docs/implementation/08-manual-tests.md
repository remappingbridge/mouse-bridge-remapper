# MBR-08 experimental — testes físicos

Placa: **Raspberry Pi Pico 2 W, RP2350, com rádio CYW43**, Waveshare Pico-LCD-1.3 240×240. O RP2350 sem o rádio da Pico 2 W não é compatível com este firmware BLE. Use `mouse-bridge-remapper-mbr08-experimental.uf2`, de produção; o firmware de qualification não testa o rádio real.

Branch exclusiva: `experimental/mbr08-integrated-recovery-20260920`, derivada do MBR-05 `7f294a7fac9eebe226ad66c6b572582c5e483421`. O arquivo `mbr08-build-manifest.json` identifica o commit, tamanho e SHA-256 do UF2. MBR-06/07/08 implementados conjuntamente por autorização do usuário. Aceitação física ainda não executada.

Gravação: desconecte a placa, mantenha BOOTSEL pressionado ao conectar USB, solte BOOTSEL e copie o UF2 para a unidade RP2350. Aguarde a reinicialização. Use um mouse BLE HOGP em modo de pareamento, fora de conexão com outro computador. Para cenários com substituição, use dois mouses, A e B. Para primeiro uso sem registros, remova os registros pela interface; não é necessário apagar toda a flash.

Execute e registre PASS/FAIL, modelo do mouse, cenário e comportamento observado. Todos os comandos da HAT agem ao soltar o botão; segurar mostra o feedback visual.

1. **Primeiro mouse:** sem registros, ligar a placa. Deve aparecer SEARCHING FIRST MOUSE; aguardar mais de 16 s antes de colocar A em pareamento. Os ciclos de 8 s continuam e A deve conectar sem reinicializar a placa.
2. **Primeira conexão:** após A qualificado, FIRST MOUSE CONNECTED. Joy e A mostram o feedback; B bloqueia, somente X desbloqueia nessa tela; Y abre HOME. Fundo didático totalmente magenta escuro.
3. **Passthrough:** testar movimento, Left, Right, Middle, Forward, Backward, roda e pan, quando disponíveis. Segurar e arrastar, depois soltar. Nenhum botão deve ficar preso ou inverter Forward/Backward.
4. **HOME:** título do mouse com sufixo MOUSE quando o nome completo não contém a palavra isolada MOUSE. As quatro opções são Pair New, resumo de remapeamento, Saved Devices e Learn. Seleção branca, demais opções e dicas cinza claro.
5. **STANDARD:** abrir o resumo, selecionar Standard, A para aplicar. Só então mostrar ativo. Conferir Left→Forward, Right→Backward, Middle→Middle, Forward→Left, Backward→Right.
6. **ESCAPE:** aplicar Escape. Conferir Left→Escape, Right→Backward, Middle→Forward, Forward→Left, Backward→Right. Verificar Escape pressionado/segurado/solto; Joy Left da tela ativa vai para HOME.
7. **Troca com botão segurado:** segurar um botão do mouse ou Escape e aplicar outro perfil com a HAT. O alvo antigo deve ser liberado. Soltar e pressionar novamente deve usar o novo perfil.
8. **Custom — editores:** abrir cada uma das cinco fontes; Up/Down escolhe um dos seis destinos. A salva o rascunho e volta para a mesma linha em EDIT CUSTOM REMAP. A mudança ainda não altera o perfil aplicado.
9. **Custom — rascunho após energia:** alterar o rascunho sem aplicar Custom, desligar/religar. O rascunho deve retornar; o perfil e template anteriormente aplicados continuam válidos.
10. **Custom — aplicar:** A em EDIT CUSTOM REMAP aplica o template inteiro; verificar as cinco saídas. Desligar/religar e confirmar perfil e template restaurados.
11. **Custom — alvo compartilhado:** mapear Left e Right para Escape. Segurar ambos; soltar um não libera Escape, soltar o último libera. Repetir com um alvo de botão de mouse.
12. **Logitech Lift:** em Standard/Custom, usar Forward como Left: pressionar, segurar arrastando e soltar. Repetir várias vezes sem cliques duplicados nem botão preso. Voltar a Passthrough e verificar Forward nativo. Repetir após reconexão.
13. **Mouse genérico:** repetir movimento, botões e perfis com um mouse sem HID++. A ausência desse recurso opcional não impede o HOGP comum.
14. **Boot com salvo:** reiniciar com A salvo e ligado. SEARCHING SAVED MOUSE deve reconectar A e restaurar o perfil confirmado, sem adicionar outro registro.
15. **Salvo ausente:** desligar A e reiniciar a placa. Em cerca de 8 s, DEVICE NOT FOUND. A reinicia uma tentativa; B durante busca cancela para DEVICE NOT FOUND.
16. **Queda na HOME:** com A conectado, desligá-lo. Liberar saídas seguradas, abrir busca de salvos automaticamente; voltar a ligar A e verificar reconexão.
17. **Queda fora da HOME:** abrir Saved Devices, desligar A. STATUS passa a DISCONNECTED e o nome deixa de ser ciano; a página permanece. B volta à HOME e inicia busca de salvos.
18. **Reconexão com página aberta:** durante busca de salvos abrir Saved Devices, ligar A. Status e cor mudam, mas a página não é substituída inesperadamente pela HOME.
19. **Help da busca:** abrir X durante busca de salvos. Voltar antes dos 8 s não reinicia o prazo. Se o prazo expirar no Help, ao voltar deve aparecer DEVICE NOT FOUND.
20. **Pair New com A ativo:** abrir Pair New pela primeira opção da HOME. Continuar movendo/clicando com A enquanto procura B; A deve permanecer funcional durante a descoberta e qualificação.
21. **Pair New ignora salvos:** com A e B já salvos, buscar novo com um deles anunciando. Nenhum salvo deve vencer como novo. A janela de 15 s permanece a mesma.
22. **Pair New cancelado/expirado:** com A funcional, cancelar com B antes de qualificar um novo; A continua. Repetir aguardando 15 s; tela de tentativa sem novo mouse, A continua. A tenta novamente.
23. **Substituição A→B novo:** deixar uma saída de A segurada e parear B ainda não salvo. A saída antiga é liberada, A desconecta e somente B passa a controlar. Ambos permanecem em Saved Devices, só B conectado/ciano.
24. **Help de Pair New:** abrir X, desligar A, voltar do Help e cancelar/voltar com B até HOME. Deve iniciar SEARCHING SAVED MOUSE. Não abrir busca de novos no lugar da busca de salvos.
25. **Perfis por mouse/template global:** salvar A com Standard e B com Escape; reconectar cada um e conferir seu perfil. Depois salvar ambos como Custom, aplicar outro template com um deles, reconectar o outro e confirmar uso do template global.
26. **Saved Devices:** percorrer com Left/Right. Conferir X OF Y, nome limitado a 21 caracteres, perfil salvo e STATUS exato. Só a página do mouse conectado tem nome ciano. Joy Press abre remoção; B volta à HOME.
27. **Remover desconectado:** com B ativo, remover A. A desaparece; B continua sem perder movimento/botões. Reiniciar e verificar que A não volta automaticamente como salvo.
28. **Remover conectado:** remover B ativo, com botão ou Escape segurado. Liberar todas as saídas, desconectar B e excluir associação/credenciais. Se restarem salvos, voltar à lista; HOME posteriormente procura os restantes.
29. **Remover último:** remover o último registro. Deve aparecer SEARCHING FIRST MOUSE. Colocar o mouse novamente em pareamento e confirmar que pode ser cadastrado como novo.
30. **Energia durante remoção:** em uma repetição de remoção, cortar energia logo após A. Ao religar, a limpeza pendente deve concluir sem registro ressuscitado nem perfil de outro mouse alterado. Registrar em que instante ocorreu o corte.
31. **Help e retorno:** conferir Help de HOME conectada, busca, retry, Pair New, retry Pair New, remapper e remoção. Texto literal correto; qualquer tecla volta ao proprietário atualizado. A seleção anterior deve ser mantida quando ainda válida.
32. **Lock comum e Learn:** onde existe a dica Y: LOCK, bloquear e desbloquear com uma tecla. A tecla que desbloqueia não executa ação extra. Mouse continua funcionando com LCD apagado. Em Learn, B bloqueia, X desbloqueia, Y abre HOME.
33. **Layout:** percorrer todas as famílias de telas. Sem sobreposição/corte vertical; título y=8, primeiro corpo y=39, rodapé ancorado em y=214. Dicas em cinza claro RGB565 0xC618, destaque de pressão branco. Fundos didáticos integrais, região magenta escura dos rodapés, sem mensagens de diagnóstico.
34. **Nomes longos:** verificar nome de mais de 21 caracteres em Saved Devices e nome com MOUSE no início/meio/final na HOME. Prefixo da HOME reservado a 15 caracteres quando precisa acrescentar o sufixo; sem MOUSE duplicado quando já existe como palavra.
35. **USB estável:** conferir VID CAFE/PID 4011, bcdDevice 0100, fabricante tiagooliveirajs, produto Mouse Bridge Remapper, sem serial. Interface 0 Mouse, interface 1 Keyboard mínima para Escape, sem CDC. Parear, remapear, remover e reconectar Bluetooth sem nova enumeração USB.
36. **Suspensão USB:** suspender/retomar o computador com A conectado e botão segurado. Retomar sem tecla presa; não desconectar/reparear BLE por causa exclusiva da mudança de estado USB.
37. **Dispositivo inadequado:** anunciar teclado BLE ou dispositivo sem coleção Mouse. Não deve virar Mouse conectado nem enviar teclas. Depois anunciar um mouse válido e verificar que a busca continua funcional.

Rollback: selecionar a branch/UF2 anterior e gravá-lo por BOOTSEL. A branch experimental não altera main. O estado de produto novo tem esquema próprio MBR8 em dois setores, separado das credenciais Bluetooth. Voltar o código não desfaz remoções/pareamentos já feitos na placa; se necessário, parear novamente. Não se declara MBR-09/10 ou aceitação física por estes testes automatizados.
