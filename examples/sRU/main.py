from jcasdata import JcasData
from time import sleep

def main():
    # init sensing-data fetcher
    fetcher = JcasData() # config see dual_mode_demo/parameters_settings.py

    # dict with fields {"th": th, "r": r, "z": z}
    data = fetcher.init_data # get "empty" data e.g. for initializing a UI with
    
    try:
        # data fetching loop
        while True:
            # fetch the data (ready for plot)
            data = fetcher.fetch()          
            #print(data)
            print("Heatmap data fetched!")
            sleep(1) # see what frequency works for your setup
    except KeyboardInterrupt:
        fetcher.fetcher_cleanup() # closes socket connection
        quit()
    except Exception as E:
        fetcher.fetcher_cleanup()
        raise E

if __name__ == "__main__":
    main()
