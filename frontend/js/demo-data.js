/* demo-data.js - fixed sample data for the online preview.
 *
 * When the C++ server cannot be reached (for example on Vercel, which only
 * hosts the HTML/CSS/JS files), app.js reads the businesses and products
 * from here instead, so browsing and search still work.
 * These are the same demo records that "grownex --seed" creates.
 * Nothing here can be changed from the website.
 */
window.GROWNEX_DEMO = {
  categories: [
    "Food & Beverages", "Clothing & Textiles", "Handicrafts", "Electronics",
    "Health & Beauty", "Home & Living", "Agriculture", "Services",
    "Stationery & Books", "Other",
  ],

  businesses: [
    {
      id: 1, ownerName: "Meera Kulkarni", businessName: "Meera's Millet Kitchen",
      category: "Food & Beverages", city: "Pune", phone: "98220 11234", memberSince: "2026-11-13",
      description: "A home kitchen turned small brand. We bake and pack millet snacks in small batches with no maida and no preservatives.",
    },
    {
      id: 2, ownerName: "Arjun Rao", businessName: "Kaveri Handloom Studio",
      category: "Clothing & Textiles", city: "Mysuru", phone: "99001 45678", memberSince: "2026-11-13",
      description: "Four weavers working on pit looms. Every piece is woven by hand from cotton yarn dyed in our own workshop.",
    },
    {
      id: 3, ownerName: "Sana Qureshi", businessName: "Leafline Repairs",
      category: "Electronics", city: "Jaipur", phone: "94140 98765", memberSince: "2026-11-13",
      description: "Phone and laptop repairs with a 90-day service warranty, plus tested refurbished accessories.",
    },
    {
      id: 4, ownerName: "Tenzin Dorje", businessName: "Bodhi Paper Co.",
      category: "Stationery & Books", city: "Bhopal", phone: "97540 22110", memberSince: "2026-11-13",
      description: "Handmade paper from cotton rag and recycled office paper. Notebooks, cards and gift wrap.",
    },
  ],

  products: [
    { id: 1, businessId: 1, name: "Ragi Jaggery Cookies (250 g)", category: "Food & Beverages", price: 180, quantity: 40, updatedAt: "2026-11-13 16:06",
      description: "Crisp finger millet cookies sweetened with jaggery. Baked fresh every Tuesday and Friday." },
    { id: 2, businessId: 1, name: "Foxtail Millet Upma Mix (500 g)", category: "Food & Beverages", price: 220, quantity: 25, updatedAt: "2026-11-13 16:06",
      description: "Roasted foxtail millet with curry leaves and mustard. Add hot water and cook for five minutes." },
    { id: 3, businessId: 1, name: "Jowar Flakes (400 g)", category: "Food & Beverages", price: 160, quantity: 0, updatedAt: "2026-11-13 16:06",
      description: "Lightly toasted sorghum flakes for breakfast. Next batch expected next week." },
    { id: 4, businessId: 2, name: "Handwoven Cotton Saree", category: "Clothing & Textiles", price: 2450, quantity: 6, updatedAt: "2026-11-13 16:06",
      description: "Six yards of soft cotton with a contrast border. Natural indigo dye, 47 inch width." },
    { id: 5, businessId: 2, name: "Cotton Stole, Checked Weave", category: "Clothing & Textiles", price: 650, quantity: 18, updatedAt: "2026-11-13 16:06",
      description: "Lightweight everyday stole, 2 m long. Available in rust, indigo and olive." },
    { id: 6, businessId: 2, name: "Table Runner (6 seater)", category: "Home & Living", price: 890, quantity: 9, updatedAt: "2026-11-13 16:06",
      description: "Handloom table runner, 72 by 14 inches. Machine washable on a gentle cycle." },
    { id: 7, businessId: 3, name: "65W Laptop Charger, Refurbished", category: "Electronics", price: 950, quantity: 12, updatedAt: "2026-11-13 16:06",
      description: "Tested original-brand charger with a new cable. Works with most Dell and HP laptops." },
    { id: 8, businessId: 3, name: "Phone Screen Replacement", category: "Services", price: 1200, quantity: 30, updatedAt: "2026-11-13 16:06",
      description: "Screen replacement for most Android phones, done the same day. Price shown is labour only." },
    { id: 9, businessId: 4, name: "Handmade Paper Notebook (A5)", category: "Stationery & Books", price: 240, quantity: 60, updatedAt: "2026-11-13 16:06",
      description: "96 unruled pages of cotton rag paper, thread bound with a soft cover." },
    { id: 10, businessId: 4, name: "Seed Paper Greeting Cards (pack of 5)", category: "Stationery & Books", price: 90, quantity: 120, updatedAt: "2026-11-13 16:06",
      description: "Plantable cards with tulsi and marigold seeds pressed into the paper." },
  ],
};
