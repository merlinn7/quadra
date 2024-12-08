import requests, re
from transformers import pipeline
from transformers import logging
import speech_recognition as sr
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.naive_bayes import MultinomialNB

dataset = [
    ("uçağı kaldır", "takeoff"),
    ("kaldır", "takeoff"),
    ("yükselt", "takeoff"),
    ("uçağı yükselt", "takeoff"),
    ("havalandır", "takeoff"),
    ("uçağı havalandır", "takeoff"),
    
    ("iniş", "landing"),
    ("yere in", "landing"),
    ("uçağı yere indir", "landing"),
    ("uçağı indir", "landing"),

    ("mod değiştir", "transition"),
    ("uçuş modu değiştir", "transition")
]

logging.set_verbosity_error()

def speech_to_text():
    recog = sr.Recognizer()
    with sr.Microphone() as mic_source:
        recog.adjust_for_ambient_noise(mic_source, duration=1)
        print("Listening...")
        audio = recog.listen(mic_source)
    try:
        speech = recog.recognize_google(audio, language="tr-TR")
        print(f"Speech Text: {speech}")
        return speech
    except sr.UnknownValueError:
        print("Speech not understood.")
        return None
    except sr.RequestError as err:
        print(f"API Error: {err}")
        return None
"""
def analyze_route(txt):
    ner_tool = pipeline("ner", model="savasy/bert-base-turkish-ner-cased")
    ner_data = ner_tool(txt)
    locs = [item["word"] for item in ner_data if item["entity"] == "B-LOC"]
    return locs

def get_coordinates(coord_name):
    try:
        url = f"https://nominatim.openstreetmap.org/search?q={coord_name}&format=json"
        headers = {"User-Agent": "my-app/1.0"}
        resp = requests.get(url, headers=headers)
        resp.raise_for_status()
        json_data = resp.json()
        if json_data:
            return json_data[0]['lat'], json_data[0]['lon']
        else:
            return None, None
    except Exception as ex:
        print(f"Error getting coordinates: {ex}")
        return None, None

def handle_route(loc_items):
    if not loc_items:
        print("No locations found.")
        return

    coords_list = []
    print("Processing Route...")
    for loc in loc_items:
        lat, lon = weird_function(loc)
        if lat and lon:
            print("Detected coordinate {0} : {1}, {2}".format(loc, lat, lon))
            coords_list.append((lat, lon))

    return coords_list
"""

def main_flow():
    user_text = speech_to_text()
    if user_text:
        return text_analysis(user_text)
    else:
        print("Text not received. Try again.")

def text_analysis(input_text):
    sentences, tags = zip(*dataset)

    vec_tool = CountVectorizer()
    feature_set = vec_tool.fit_transform(sentences) 

    model_f = MultinomialNB()
    model_f.fit(feature_set, tags) 

    def inner_parser(message):
        feature_check = vec_tool.transform([message])
        pred_command = model_f.predict(feature_check)[0]
        num = 0
        if pred_command == "takeoff":
            numbers = re.findall(r'\d+', message) 
            if numbers: 
                num = int(numbers[-1]) 
        elif pred_command == "landing":
            pass
        return pred_command, num

    cmd, value = inner_parser(input_text)
    return cmd, value
