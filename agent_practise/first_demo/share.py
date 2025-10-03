from smolagents import CodeAgent, DuckDuckGoSearchTool, InferenceClientModel

from huggingface_hub import login
login()

agent = CodeAgent(tools=[DuckDuckGoSearchTool()], model=InferenceClientModel())

agent.run("给我一些学习agent的建议，使用中文回答")

agent.push_to_hub('xdargs/first_demo')