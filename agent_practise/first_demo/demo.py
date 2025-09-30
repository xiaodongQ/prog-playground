from smolagents import CodeAgent, DuckDuckGoSearchTool, InferenceClientModel

agent = CodeAgent(tools=[DuckDuckGoSearchTool()], model=InferenceClientModel())
agent.run("Search for the best music recommendations for a party at the Wayne's mansion.")
