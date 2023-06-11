import pyttsx3
from rasa.core.run import serve_application
from rasa.core.agent import Agent
from rasa.utils.endpoints import EndpointConfig


def speak(response):
    engine.say(response)
    engine.runAndWait()


def chat_with_bot():
    while True:
        user_input = input("User: ")
        responses = agent.handle_text(user_input)
        for response in responses:
            print("Bot:", response["text"])
            speak(response["text"])


# Initialize the text-to-speech engine
engine = pyttsx3.init()

# Load the Rasa agent
config = "config.yml"  # Path to your Rasa config file
model = "models"  # Path to your trained Rasa model directory
endpoints = EndpointConfig("endpoints.yml")  # Path to your Rasa endpoints file
agent = Agent.load(model, domain="domain.yml", config="config.yml",
                   action_endpoint=endpoints.action)

# Start the conversation loop
chat_with_bot()
