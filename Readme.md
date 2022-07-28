# RECOVERY POWER MINIPC CHUWI COREBOX

## ANTECEDENTES

### Tengo un miniPC Chuwi Corebox i5 que no permite el auto arranque mediante la configuración de la BIOS. Por lo visto si permite el arranque mediante "wake on lan" (lo tengo pendiente de probar). Al no permitir el auto arranque tras pérdida de energía el miniPC no podría usarse como centro de Control Domótico (Home Assistant)

## DESCRIPCIÓN DEL PROYECTO

### Material necesario

* 1 x Wemos D1 mini (microcontrolador) ~ 3€
* 1 x Servomotor Digital SG90 MG90 MG90S ~ 3€
* 1 x F.A 5v 600~700 mA (Fuente de alimentación) ~ 3€
* Varios: Tope de silicona, impresora 3D, placa electrónica soldar y bridas unex

### Mediante este dispositivo se le proporciona al miniPC Chuwi la posibilidad de arrancar tras la perdida y recuperación de la emnrgía para así poderse utilizar este miniPC como Centro Domótico de Control. El dispositivo diseñado realiza el auto arranque tras perdida de energía mediante un servo motor que pulsa el interruptor mecánico del miniPC y lo pone en funcionamiento. Cada vez que se reinicie el dispositivo por pérdida de energía arrancará el miniPC. Además mediante mqtt, puedes controlar el servo motor con unas serie de órdenes en el topic "Casa/Autopower/Control"

* "reinicia". Si ponemos esta orden en el topic "Casa/Autopower/Control", el servo mantiene pulsado el interruptor del miniPC Chuwi durante 5 segundos
* "reset". Si ponemos esta orden en el topic "Casa/Autopower/Control", el servo mantiene pulsado el interruptor del miniPC Chuwi durante 1 segundo, lo mismo que si ocurriera una pérdida de tensión y una recuperación posterior

### También disponemos de los topic "Casa/Autopower/Estado" y "Casa/Autopower/Testamento" para saber el estado del dispositivo


## Realizado por gurues (gurues@3DO ~ 2022 ~ ogurues@gmail.com)
