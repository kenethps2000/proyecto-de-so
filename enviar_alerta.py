# enviar_alerta.py

from twilio.rest import Client
import sys

# Obtener los parámetros de entrada
numero_destino = sys.argv[1]  # Número de WhatsApp de destino
mensaje = sys.argv[2]  # El mensaje a enviar

# Credenciales de Twilio
sid = 'ACcc631bfb31f0c1b9b136db7ba35a2208'  # Sustituye con tu SID de Twilio
token = 'd3b0a0cecdc1d413bab6c8514712aa8a'  # Sustituye con tu Token de autenticación
from_number = 'whatsapp:+14155238886'  # El número de Twilio para WhatsApp (prueba en sandbox o en producción)

# Crear el cliente de Twilio
client = Client(sid, token)

# Enviar el mensaje
message = client.messages.create(
    body=mensaje,
    from_=from_number,
    to=numero_destino
)

