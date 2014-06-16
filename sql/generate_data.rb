require 'csv'

URLS = ['http://www.google.com', 'http://www.cnn.com', 'http://www.yahoo.com', 'http://www.abc.com', 'http://www.wikipedia.org']
ROW_COUNT = 50000
USER_COUNT = 10000
HOURS = 60*60
DAYS = 24 * HOURS

CSV.open('data.csv', 'w') do |csv|
  ROW_COUNT.times do 
    csv << [URLS.sample, (1..USER_COUNT).to_a.sample, Time.now - (0..7).to_a.sample * DAYS - (0..12).to_a.sample * HOURS, (0..60).to_a.sample]
  end
end
