
"""
This file is to try to run home assistant as a service.
"""
import logging
import time

logger = logging.getLogger(__name__)

logging.basicConfig(filename='logs/core.log',
                    level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - [home_assistant] - %(message)s')

if __name__ == '__main__':
    logger.info('Home Assistant started')
    while True:
        logger.info('Home Assistant running')
        time.sleep(10)