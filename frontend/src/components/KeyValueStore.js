import React, { useState, useEffect } from 'react';

const KeyValueStore = () => {
  const [pairs, setPairs] = useState([]);
  const [newKey, setNewKey] = useState('');
  const [newValue, setNewValue] = useState('');
  const [incrementKey, setIncrementKey] = useState('');
  const [incrementAmount, setIncrementAmount] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const API_URL = 'http://localhost:8080';

  const fetchPairs = async () => {
    try {
      setLoading(true);
      const response = await fetch(`${API_URL}/api/keys`);
      const data = await response.json();
      setPairs(data.pairs || []);
      setError('');
    } catch (err) {
      setError('Failed to fetch key-value pairs');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchPairs();
  }, []);

  const handleAddPair = async (e) => {
    e.preventDefault();
    if (!newKey || !newValue) {
      setError('Both key and value are required');
      return;
    }

    try {
      const response = await fetch(`${API_URL}/api/keys`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ key: newKey, value: parseInt(newValue, 10) })
      });

      if (!response.ok) throw new Error('Failed to add key-value pair');

      setNewKey('');
      setNewValue('');
      setError('');
      fetchPairs();
    } catch (err) {
      setError(err.message);
    }
  };

  const handleDelete = async (key) => {
    try {
      const response = await fetch(`${API_URL}/api/keys/${key}`, {
        method: 'DELETE'
      });

      if (!response.ok) throw new Error('Failed to delete key');

      fetchPairs();
    } catch (err) {
      setError(err.message);
    }
  };

  const handleIncrement = async (e) => {
    e.preventDefault();
    if (!incrementKey || !incrementAmount) {
      setError('Both key and increment amount are required');
      return;
    }

    try {
      const response = await fetch(`${API_URL}/api/keys/${incrementKey}/increment`, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ amount: parseInt(incrementAmount, 10) })
      });

      if (!response.ok) throw new Error('Failed to increment value');

      setIncrementKey('');
      setIncrementAmount('');
      setError('');
      fetchPairs();
    } catch (err) {
      setError(err.message);
    }
  };

  return (
    <div className="max-w-4xl mx-auto p-4">
      <div className="bg-white rounded-lg shadow-lg p-6">
        <div className="flex justify-between items-center mb-6">
          <h1 className="text-2xl font-bold">Password Manager</h1>
          <button 
            onClick={fetchPairs}
            disabled={loading}
            className="px-4 py-2 rounded bg-blue-500 text-white hover:bg-blue-600 disabled:opacity-50"
          >
            {loading ? 'Refreshing...' : 'Refresh'}
          </button>
        </div>

        {error && (
          <div className="mb-4 p-4 bg-red-100 border border-red-400 text-red-700 rounded">
            {error}
          </div>
        )}

        <div className="space-y-6">
          {/* Add new key-value pair */}
          <form onSubmit={handleAddPair} className="flex gap-2">
            <input
              type="text"
              placeholder="Username"
              value={newKey}
              onChange={(e) => setNewKey(e.target.value)}
              className="flex-1 px-4 py-2 border rounded focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
            <input
              type="number"
              placeholder="Password"
              value={newValue}
              onChange={(e) => setNewValue(e.target.value)}
              className="flex-1 px-4 py-2 border rounded focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
            <button 
              type="submit"
              className="px-4 py-2 bg-green-500 text-white rounded hover:bg-green-600"
            >
              Add
            </button>
          </form>

          {/* Increment value */}
          <form onSubmit={handleIncrement} className="flex gap-2">
            <input
              type="text"
              placeholder="Username to increment"
              value={incrementKey}
              onChange={(e) => setIncrementKey(e.target.value)}
              className="flex-1 px-4 py-2 border rounded focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
            <input
              type="number"
              placeholder="Increment password by"
              value={incrementAmount}
              onChange={(e) => setIncrementAmount(e.target.value)}
              className="flex-1 px-4 py-2 border rounded focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
            <button 
              type="submit"
              className="px-4 py-2 bg-purple-500 text-white rounded hover:bg-purple-600"
            >
              Increment
            </button>
          </form>

          {/* Key-value pairs list */}
          <div className="border rounded-lg">
            <div className="grid grid-cols-3 gap-4 px-4 py-2 font-medium bg-gray-50">
              <div>Key</div>
              <div>Value</div>
              <div>Actions</div>
            </div>
            {pairs.map((pair) => (
              <div key={pair.key} className="grid grid-cols-3 gap-4 px-4 py-2 items-center border-t">
                <div className="font-medium">{pair.key}</div>
                <div>{pair.value}</div>
                <div>
                  <button
                    onClick={() => handleDelete(pair.key)}
                    className="px-3 py-1 text-red-600 hover:bg-red-50 rounded"
                  >
                    Delete
                  </button>
                </div>
              </div>
            ))}
            {pairs.length === 0 && (
              <div className="px-4 py-8 text-center text-gray-500 border-t">
                No key-value pairs found
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

export default KeyValueStore;