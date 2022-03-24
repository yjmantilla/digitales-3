class StepperMotor(object):
    def __init__(self) -> None:
        self.list_directions = ['clockwise','counter-clockwise']
        self.dict_directions = {x:self.list_directions.index(x) for x in self.list_directions}