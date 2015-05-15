class site:
    name=''
    host=''
    port=1094
    def __init__(self, na, ho):
        self.name=na
        ho=ho.replace("root://","")
        self.host=ho.split(":")[0]
        if ho.count(":"):
            self.port=ho.split(":")[1]
        self.ddms=[]
    def prnt(self):
        logging.debug('name: %s \thost: %s:%s' % (self.name, self.host, self.port ))
        for i in range(len(self.ddms)):
            logging.debug(self.ddms[i])


