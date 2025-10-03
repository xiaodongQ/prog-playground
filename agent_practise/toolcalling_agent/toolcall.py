from smolagents import ToolCallingAgent, DuckDuckGoSearchTool, InferenceClientModel

agent = ToolCallingAgent(tools=[DuckDuckGoSearchTool()], model=InferenceClientModel())

agent.run("Search for the best music recommendations for a party at the Wayne's mansion.")