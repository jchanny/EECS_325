#Jeremy Chan, jsc126
#python script that parses text file, performs analysis and creates graph
import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
plt.style.use('ggplot')

df = pd.read_csv(sys.argv[1], sep=" ", header=None)
df.columns = ["domain", "min", "avg", "max", "mdev"]

#remove NaN columns
df = df[pd.notnull(df["min"])]

avg_std_dev = df["mdev"].mean()
avg_time = df["avg"].mean()

print( "Average time taken:", avg_time)
print( "Average standard deviation:", avg_std_dev)

social_media = ['facebook.com', 'twitter.com', 'linkedin.com', 'instagram.com', 't.co', 'pinterest.com']
video = ['youtube.com', 'twitch.tv', 'pornhub.com', 'cnn.com']
shopping = ['amazon.com', 'paypal.com', 'craigslist.com']
news = ['reddit.com', 'wikipedia.org', 'yahoo.com', 'cnn.com', 'nytimes.com']
lifestyle = ['google.com', 'yahoo.com', 'live.com', 'bing.com', 'instructure.com']

social_media_time = 0
social_media_dev= 0
video_time= 0
video_dev= 0
shopping_time = 0
shopping_dev= 0
news_time= 0
news_dev= 0
lifestyle_time = 0
lifestyle_dev = 0

for index, row in df.iterrows():
    if row["domain"] in social_media:
        social_media_time += row["avg"]
        social_media_dev += row["mdev"]
    if row["domain"] in video:
        video_time += row["avg"]
        video_dev += row["mdev"]
    if row["domain"] in shopping:
        shopping_time += row["avg"]
        shopping_dev += row["mdev"]
    if row["domain"] in news:
        news_time += row["avg"]
        news_dev += row["mdev"]
    if row["domain"] in lifestyle:
        lifestyle_time += row["avg"]
        lifestyle_dev += row["mdev"]
           
print( "Social media average time taken: ",(social_media_time/len(social_media)), "+/-", (social_media_dev/len(social_media)))
print( "Video site average time taken: ", (video_time/len(video)), "+/-", (video_dev/len(video)))
print( "Shopping site average time taken: ", (shopping_time/len(shopping)), "+/-", (shopping_dev/len(shopping)))
print( "News site average time taken: ", (news_time/len(news)), "+/-", (news_dev/len(news)))
print( "Lifestyle site average time taken: ", (lifestyle_time/len(lifestyle)), "+/-", (lifestyle_dev/len(lifestyle)))

#bar graph for avg time w/error bars from mdev classified by category
x = ['Social Media', 'Video', 'Shopping', 'News', 'Lifestyle']
time = [(social_media_time/len(social_media)), (video_time/len(video)), (shopping_time/len(shopping)), (news_time/len(news)), (lifestyle_time/len(lifestyle))]
deviation = [(social_media_dev/len(social_media)), (video_dev/len(video)), (shopping_dev/len(shopping)), (news_dev/len(news)), (lifestyle_dev/len(lifestyle))]

x_pos = [i for i, _ in enumerate(x)]
plt.bar(x_pos, time, color='green', yerr=deviation)
plt.title("Average RTT by Website Category")
plt.xlabel("Website category")
plt.ylabel("Average time in ms")
plt.xticks(x_pos, x)
plt.show()
