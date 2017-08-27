window.addEvent('domready', function() {
	$$('.focus').each(function(e){e.focus();});
	$$('table.list tr:nth-child(2n) td').each(function(e){e.set('class', 'even');});
	$$('table.list tr:nth-child(2n+1) td').each(function(e){e.set('class', 'odd');});
	$$('table.list tr th').each(function(e){e.set('class', 'head');});
	$$('table.list tr td').each(function(e){e.addEvents({
		'click': function(){
		    location.href = this.getChildren('a')[0].get('href');
		}
	})});
});


